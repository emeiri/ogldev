# Golden-image regression runner, for Windows and Linux.
#
#   python Testing/regress.py                       run every 'ok' row of Testing/manifest.csv
#   python Testing/regress.py --only Name1 Name2     run a subset (also rows that are still 'todo')
#   python Testing/regress.py --update-golden --only Name1
#   python Testing/regress.py --coverage
#
# The runner detects the computer it runs on from Testing/machines.csv (or
# --machine NAME). Each project is launched in its test mode (see
# Include/regress_hook.h), the frames it captured are compared strictly with the
# goldens of this machine, Testing/goldens/<machine>/<project>/fNNNN.png, and with
# the goldens of every other machine using a larger tolerance. The result goes to
# Testing/out/report.html and Testing/out/results.json, every run leaves a
# readable Testing/out/run_<date>_<time>.log with the status of each project, and
# Testing/results/<machine>.md summarizes the run per golden set (to commit).
#
# On Linux a project is captured either on the GPU of the desktop session
# ('hw') or with Mesa's software renderers on a virtual X server ('sw').
# The method is chosen when the golden is first created and recorded in its
# golden.json, and later runs on that machine always use it.
#
# Needs: pip install pillow numpy (on Linux: apt install python3-pil python3-numpy xvfb mesa-utils)

import argparse, atexit, csv, datetime, html, json, os, re, shutil, signal, socket, subprocess, sys, time
from pathlib import Path
import numpy as np
from PIL import Image

REPO = Path(__file__).resolve().parent.parent
TESTING = REPO / "Testing"
MANIFEST = TESTING / "manifest.csv"
MACHINES = TESTING / "machines.csv"
OUT, GOLD, RESULTS = TESTING / "out", TESTING / "goldens", TESTING / "results"
DLL_DIR = REPO / "Windows" / "DLL"
DEFAULT_SLN = REPO / "Windows" / "ogldev_vs_2026" / "ogldev_vs_2026.sln"
DEFAULT_TOL, DEFAULT_MAX_BAD, DEFAULT_TIMEOUT = 8, 0.001, 120
# Comparison with the goldens of the other machines (different GPU, driver and maybe OS)
DEFAULT_CROSS_TOL, DEFAULT_CROSS_MAX_BAD = 48, 0.02
EXIT_CODES = {2: "capture failed", 3: "API error in strict mode"}

PLATFORM = "linux" if sys.platform.startswith("linux") else "windows"
# --window: the size of the OpenGL windows in test mode (--test-window). By default
# every project keeps the size it asks for, because many tutorials size their
# viewport and projection from their own constants and draw only part of the
# image into a smaller window. The comparison with the other machines scales
# the larger image down.
WINDOW = ""
# This machine (a row of machines.csv) and its golden set, set by main()
MACHINE, OWN = {}, ""
# Every golden set: [{"name", "platform", "description"}], this machine's first
SETS = []
STATUS = "linux_status" if PLATFORM == "linux" else "status"
NOTE = "linux_note" if PLATFORM == "linux" else "note"

# Linux capture methods, in the order they are tried when a golden is first created
#   hw      the GPU, on the X display of the desktop session
#   sw      Mesa llvmpipe (OpenGL) and lavapipe (Vulkan) on a virtual X server
#   sw-gl46 the same, with llvmpipe reporting OpenGL 4.6 (it implements 4.5)
METHODS = ["hw", "sw", "sw-gl46"]
SOFTWARE_RENDERERS = ("llvmpipe", "softpipe", "lavapipe", "swrast", "software")
LAVAPIPE_ICD = "/usr/share/vulkan/icd.d/lvp_icd.json"
XVFB_SIZE = (2560, 1600)


# ---------------------------------------------------------------------------
#  Platform specific details of a manifest row
# ---------------------------------------------------------------------------

def on_platform(row, platform=PLATFORM):
    return bool(row.get("linux_dir")) if platform == "linux" else bool(row.get("exe"))


_targets = {}


def linux_target(linux_dir):
    # The executable is the TARGET of the project's Makefile
    if linux_dir not in _targets:
        target = ""
        try:
            m = re.search(r"^TARGET\s*:?=\s*(\S+)", (REPO / linux_dir / "Makefile").read_text(), re.M)
            target = m.group(1) if m else ""
        except OSError:
            pass
        _targets[linux_dir] = target
    return _targets[linux_dir]


def exe_path(row):
    if PLATFORM == "linux":
        return REPO / row["linux_dir"] / linux_target(row["linux_dir"])
    return REPO / row["exe"]


def cwd_path(row):
    if PLATFORM == "linux":
        return REPO / (row.get("linux_cwd") or row["linux_dir"])
    return REPO / row["cwd"]


def args_of(row):
    args = row["args"].split()
    return [a.replace("\\", "/") for a in args] if PLATFORM == "linux" else args


# ---------------------------------------------------------------------------
#  Machines and golden sets
# ---------------------------------------------------------------------------

def read_machines():
    try:
        with open(MACHINES, newline="") as f:
            return [m for m in csv.DictReader(f) if m.get("name")]
    except OSError:
        return []


def hardware_product():
    # The vendor and model of the computer as its firmware reports them
    parts = []
    if PLATFORM == "linux":
        for key in ("sys_vendor", "product_name", "product_version", "product_family"):
            try:
                parts.append((Path("/sys/class/dmi/id") / key).read_text().strip())
            except OSError:
                pass
    else:
        try:
            import winreg
            with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, r"HARDWARE\DESCRIPTION\System\BIOS") as k:
                for value in ("SystemManufacturer", "SystemProductName", "SystemVersion", "SystemFamily"):
                    try:
                        parts.append(str(winreg.QueryValueEx(k, value)[0]).strip())
                    except OSError:
                        pass
        except OSError:
            pass
    return " ".join(p for p in parts if p)


def detect_machine(machines, name):
    # Returns (machines.csv row, how it was found). A row matches when its 'product'
    # is part of the firmware's vendor and model, or else by its number of logical
    # CPUs. A platform with a single machine and no criteria needs neither.
    known = ", ".join(m["name"] for m in machines) or "none"
    if name:
        m = next((m for m in machines if m["name"] == name), None)
        if m is None:
            sys.exit(f"machine '{name}' is not in {MACHINES.relative_to(REPO)} (known: {known})")
        if m["platform"] != PLATFORM:
            sys.exit(f"machine '{name}' is a {m['platform']} machine, this is {PLATFORM}")
        return m, "chosen with --machine or OGLDEV_MACHINE"
    candidates = [m for m in machines if m["platform"] == PLATFORM]
    if len(candidates) == 1 and not candidates[0].get("product") and not candidates[0].get("cpus"):
        return candidates[0], f"the only {PLATFORM} machine"
    product, cpus = hardware_product(), os.cpu_count()
    by_product = [m for m in candidates if m.get("product") and m["product"].lower() in product.lower()]
    if len(by_product) == 1:
        return by_product[0], f"model '{product}'"
    by_cpus = [m for m in (by_product or candidates) if m.get("cpus") and int(m["cpus"]) == cpus]
    if len(by_cpus) == 1:
        return by_cpus[0], f"{cpus} logical CPUs"
    sys.exit(f"cannot tell which machine this is (model '{product or 'unknown'}', {cpus} logical CPUs).\n"
             f"Add it to {MACHINES.relative_to(REPO)} or pass --machine NAME (known: {known})")


def golden_sets(machines):
    # This machine first, then the others in the order of machines.csv, then any
    # other folder of Testing/goldens
    sets = [{"name": m["name"], "platform": m["platform"], "description": m.get("description", "")}
            for m in machines]
    names = {s["name"] for s in sets}
    if GOLD.exists():
        for d in sorted(GOLD.iterdir()):
            if d.is_dir() and d.name not in names:
                sets.append({"name": d.name, "platform": "linux" if d.name.startswith("linux") else "windows",
                             "description": ""})
    return sorted(sets, key=lambda s: s["name"] != OWN)


# ---------------------------------------------------------------------------
#  Linux displays: the desktop session for 'hw', a private Xvfb for 'sw'
# ---------------------------------------------------------------------------

class LinuxDisplays:
    def __init__(self, timeout):
        self.timeout = timeout
        self.xvfb, self.sw_display = None, None
        self.hw_checked, self.hw_display, self.hw_renderer, self.hw_reason = False, None, "", ""
        self.xset_restore, self.scale_restore = None, None
        atexit.register(self.close)

    def software(self):
        if self.xvfb is None:
            if not shutil.which("Xvfb"):
                sys.exit("Xvfb is not installed (sudo apt install xvfb)")
            for n in range(90, 110):
                if not Path(f"/tmp/.X11-unix/X{n}").exists() and not Path(f"/tmp/.X{n}-lock").exists():
                    break
            # Large enough for the projects that ask for big windows
            self.xvfb = subprocess.Popen(["Xvfb", f":{n}", "-screen", "0", f"{XVFB_SIZE[0]}x{XVFB_SIZE[1]}x24", "-nolisten", "tcp"],
                                         stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            for _ in range(100):
                if Path(f"/tmp/.X11-unix/X{n}").exists():
                    break
                time.sleep(0.1)
            self.sw_display = f":{n}"
        return self.sw_display

    def hardware(self):
        # Returns the X display of the desktop session if its GPU renders, else None (see hw_reason)
        if self.hw_checked:
            return self.hw_display
        self.hw_checked = True
        display = os.environ.get("OGLDEV_HW_DISPLAY") or os.environ.get("DISPLAY") or ":0"
        env = self.env("hw", display)
        if not shutil.which("glxinfo"):
            self.hw_reason = "glxinfo is missing (sudo apt install mesa-utils)"
            return None
        try:
            r = subprocess.run(["glxinfo", "-B"], env=env, capture_output=True, text=True, timeout=10)
        except subprocess.TimeoutExpired:
            self.hw_reason = f"the X display {display} does not respond (is the session locked or on another console?)"
            return None
        m = re.search(r"OpenGL renderer string:\s*(.*)", r.stdout)
        if r.returncode != 0 or not m:
            self.hw_reason = f"cannot use the X display {display}: {(r.stderr or r.stdout).strip()[:200]}"
            return None
        renderer = m.group(1).strip()
        if is_software(renderer):
            self.hw_reason = f"the X display {display} renders in software ({renderer})"
            return None
        self.hw_display, self.hw_renderer = display, renderer
        self.keep_awake(env)
        self.enlarge_screen(env)
        return display

    def enlarge_screen(self, env):
        # The window manager shrinks a window that doesn't fit the work area (a 1920x1080
        # window on a 1920x1080 screen loses the panel and the title bar), which changes
        # the image. Scale the X screen up to the size of the virtual X server for the
        # run: the GPU still renders the windows at their real size. OGLDEV_HW_SCALE=0
        # turns it off.
        if os.environ.get("OGLDEV_HW_SCALE") == "0" or not shutil.which("xrandr"):
            return
        q = subprocess.run(["xrandr", "--current"], env=env, capture_output=True, text=True).stdout
        out = re.search(r"^(\S+) connected primary (\d+)x(\d+)", q, re.M) or \
              re.search(r"^(\S+) connected (\d+)x(\d+)", q, re.M)
        if not out or len(re.findall(r"^\S+ connected (?:primary )?\d", q, re.M)) != 1:
            return      # nothing to do, or several monitors: leave the layout alone
        name, w, h = out.group(1), int(out.group(2)), int(out.group(3))
        scale = max(XVFB_SIZE[0] / w, XVFB_SIZE[1] / h)
        if scale <= 1:
            return
        r = subprocess.run(["xrandr", "--output", name, "--scale", f"{scale:.4f}x{scale:.4f}"],
                           env=env, capture_output=True, text=True)
        if r.returncode == 0:
            self.scale_restore = (env, ["xrandr", "--output", name, "--scale", "1x1"])
            time.sleep(3)   # let the window manager take the new work area

    def keep_awake(self, env):
        # A screen saver would lock the session (light-locker), which stops the GPU
        # rendering for it. Switch it and DPMS off for the run and restore them afterwards.
        if not shutil.which("xset"):
            return
        q = subprocess.run(["xset", "q"], env=env, capture_output=True, text=True).stdout
        saver = re.search(r"timeout:\s*(\d+)\s+cycle:\s*(\d+)", q)
        dpms = re.search(r"Standby:\s*(\d+)\s+Suspend:\s*(\d+)\s+Off:\s*(\d+)", q)
        restore = []
        if saver:
            restore.append(["xset", "s", saver.group(1), saver.group(2)])
        if dpms:
            restore.append(["xset", "dpms", dpms.group(1), dpms.group(2), dpms.group(3)])
            restore.append(["xset", "+dpms" if "DPMS is Enabled" in q else "-dpms"])
        subprocess.run(["xset", "s", "off", "-dpms"], env=env, capture_output=True)
        self.xset_restore = (env, restore)

    def env(self, method, display):
        env = dict(os.environ)
        env["DISPLAY"] = display
        for var in ("LIBGL_ALWAYS_SOFTWARE", "GALLIUM_DRIVER", "MESA_GL_VERSION_OVERRIDE",
                    "MESA_GLSL_VERSION_OVERRIDE", "VK_DRIVER_FILES", "VK_ICD_FILENAMES"):
            env.pop(var, None)
        if method == "hw":
            env.setdefault("XAUTHORITY", str(Path.home() / ".Xauthority"))
            return env
        env.pop("XAUTHORITY", None)
        env.update(LIBGL_ALWAYS_SOFTWARE="1", GALLIUM_DRIVER="llvmpipe")
        if Path(LAVAPIPE_ICD).exists():
            env.update(VK_DRIVER_FILES=LAVAPIPE_ICD, VK_ICD_FILENAMES=LAVAPIPE_ICD)
        if method == "sw-gl46":
            env.update(MESA_GL_VERSION_OVERRIDE="4.6", MESA_GLSL_VERSION_OVERRIDE="460")
        return env

    def for_method(self, method):
        # (environment, None) or (None, reason why the method is not available)
        if method == "hw":
            display = self.hardware()
            return (self.env("hw", display), None) if display else (None, self.hw_reason)
        return self.env(method, self.software()), None

    def close(self):
        if self.scale_restore:
            env, cmd = self.scale_restore
            subprocess.run(cmd, env=env, capture_output=True)
            self.scale_restore = None
        if self.xset_restore:
            env, restore = self.xset_restore
            for cmd in restore:
                subprocess.run(cmd, env=env, capture_output=True)
            self.xset_restore = None
        if self.xvfb:
            self.xvfb.terminate()
            try:
                self.xvfb.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.xvfb.kill()
            self.xvfb = None


def is_software(renderer):
    return any(s in renderer.lower() for s in SOFTWARE_RENDERERS)


# ---------------------------------------------------------------------------
#  Manifest
# ---------------------------------------------------------------------------

def find_msbuild():
    if os.environ.get("MSBUILD"):
        return os.environ["MSBUILD"]
    vswhere = Path(os.environ["ProgramFiles(x86)"]) / "Microsoft Visual Studio" / "Installer" / "vswhere.exe"
    r = subprocess.run([str(vswhere), "-latest", "-prerelease", "-requires", "Microsoft.Component.MSBuild",
                        "-find", r"MSBuild\**\Bin\MSBuild.exe"], capture_output=True, text=True)
    lines = r.stdout.split("\n")
    return lines[0].strip() if lines[0].strip() else "msbuild"


def linux_projects():
    # Directories whose Makefile includes Make/project.mk
    found = []
    for d, dirs, files in os.walk(REPO):
        dirs[:] = [x for x in dirs if x not in (".git", "build")]
        if "Makefile" in files and Path(d) != REPO:
            try:
                if "Make/project.mk" in (Path(d) / "Makefile").read_text():
                    found.append(Path(d).relative_to(REPO).as_posix())
            except OSError:
                pass
    return sorted(found)


def missing_from_manifest(rows, sln=DEFAULT_SLN):
    # Application projects that have no manifest row, so they are never tested
    if PLATFORM == "linux":
        known = {r.get("linux_dir", "") for r in rows}
        return [d for d in linux_projects() if d not in known]
    known = {r.get("project", "").lower() for r in rows}
    missing = []
    try:
        text = Path(sln).read_text(encoding="utf-8-sig")
    except OSError:
        return missing
    for m in re.finditer(r'Project\("\{[^}]+\}"\) = "([^"]+)", "([^"]+\.vcxproj)"', text):
        proj = Path(os.path.normpath(Path(sln).resolve().parent / m.group(2)))
        try:
            rel = proj.relative_to(REPO).as_posix()
            is_app = "<ConfigurationType>Application" in proj.read_text(encoding="utf-8-sig", errors="replace")
        except (OSError, ValueError):
            continue
        if is_app and rel.lower() not in known:
            missing.append(f"{m.group(1)} ({rel})")
    return missing


def manifest_warning(missing):
    if not missing:
        return []
    if PLATFORM == "linux":
        how = ["  To add them: python3 Testing/make_manifest.py --linux",
               "  then try each one with --only and set its linux_status to 'ok' (see Testing/README.md)."]
    else:
        how = ["  To add them: python Testing\\make_manifest.py Windows\\ogldev_vs_2026\\ogldev_vs_2026.sln",
               "  then try each one with --only and set its status to 'ok' (see Testing\\README.md)."]
    return [f"WARNING: {len(missing)} project(s) are missing from the manifest and are not tested:"] + \
           [f"  {m}" for m in missing] + how


def write_manifest(rows):
    # Keep the line endings of the file so that only the edited rows show up in a diff
    newline = "\r\n" if b"\r\n" in MANIFEST.read_bytes() else "\n"
    with open(MANIFEST, "w", newline="") as f:
        w = csv.DictWriter(f, list(rows[0].keys()), lineterminator=newline)
        w.writeheader()
        w.writerows(rows)


def parse_masks(text):
    return [tuple(int(v) for v in m.split(",")) for m in text.split(";") if m.strip()]


def frame_list(row):
    return [int(f) for f in (row["frames"] or "10").split(";") if f.strip()]


# ---------------------------------------------------------------------------
#  Running and comparing
# ---------------------------------------------------------------------------

def load(path, masks):
    a = np.asarray(Image.open(path).convert("RGB"), dtype=np.int16).copy()
    for x, y, w, h in masks:
        a[y:y + h, x:x + w] = 0
    return a


def compare(actual, golden, tol, max_bad, masks):
    a, g = load(actual, masks), load(golden, masks)
    if a.shape != g.shape:
        return False, 1.0, None
    diff = np.abs(a - g).max(axis=2)
    bad = float((diff > tol).mean())
    return bad <= max_bad, bad, np.clip(diff * 8, 0, 255).astype(np.uint8)


def resized(a, w, h):
    return np.asarray(Image.fromarray(a.astype(np.uint8)).resize((w, h), Image.BOX), dtype=np.int16)


def crop_to_aspect(a, aspect, keep):
    # Center crop to the aspect ratio width/height. keep="height" removes columns
    # (a projection with a fixed vertical field of view), keep="width" rows.
    h, w = a.shape[:2]
    if keep == "height" and w / h > aspect:
        cw = max(1, round(h * aspect))
        x = (w - cw) // 2
        return a[:, x:x + cw]
    if keep == "width" and w / h < aspect:
        ch = max(1, round(w / aspect))
        y = (h - ch) // 2
        return a[y:y + ch]
    return a


def fitted(a, g):
    # Images of different sizes, made the same size: the larger one is scaled down
    # to the smaller one. A different aspect ratio is first removed by cropping
    # (the projection keeps its vertical or its horizontal field of view) or by
    # stretching (no projection, e.g. the first tutorials). Yields (how, a, g).
    (ah, aw), (gh, gw) = a.shape[:2], g.shape[:2]
    candidates = [("scaled", a, g)]
    if aw * gh != gw * ah:
        aspect = min(aw / ah, gw / gh)
        candidates.append(("cropped", crop_to_aspect(a, aspect, "height"), crop_to_aspect(g, aspect, "height")))
        aspect = max(aw / ah, gw / gh)
        candidates.append(("cropped", crop_to_aspect(a, aspect, "width"), crop_to_aspect(g, aspect, "width")))
    for how, ca, cg in candidates:
        w, h = min(ca.shape[1], cg.shape[1]), min(ca.shape[0], cg.shape[0])
        yield how, (ca if ca.shape[:2] == (h, w) else resized(ca, w, h)), (cg if cg.shape[:2] == (h, w) else resized(cg, w, h))


def compare_cross(actual, golden, tol, max_bad, masks):
    # The comparison with another machine's golden, which may have another size.
    # Returns (ok, fraction of bad pixels, difference image, note).
    a, g = load(actual, masks), load(golden, masks)
    if a.shape == g.shape:
        diff = np.abs(a - g).max(axis=2)
        bad = float((diff > tol).mean())
        return bad <= max_bad, bad, np.clip(diff * 8, 0, 255).astype(np.uint8), ""
    best = None
    for how, fa, fg in fitted(a, g):
        diff = np.abs(fa - fg).max(axis=2)
        bad = float((diff > tol).mean())
        if best is None or bad < best[0]:
            best = (bad, diff, how)
    bad, diff, how = best
    note = f"{how}: {a.shape[1]}x{a.shape[0]} and {g.shape[1]}x{g.shape[0]} compared at {diff.shape[1]}x{diff.shape[0]}"
    return bad <= max_bad, bad, np.clip(diff * 8, 0, 255).astype(np.uint8), note


def run_once(row, outdir, strict, timeout, env=None):
    exe = exe_path(row)
    if not exe.is_file():
        return "not built"
    # A process that has just exited may still hold the directory for a moment
    for _ in range(20):
        shutil.rmtree(outdir, ignore_errors=True)
        if not outdir.exists():
            break
        time.sleep(0.1)
    outdir.mkdir(parents=True, exist_ok=True)
    for stale in outdir.glob("*"):
        stale.unlink()
    # The project's own arguments go first because some projects read argv[1]
    cmd = [str(exe)] + args_of(row)
    cmd += ["--test-frames", ",".join(str(f) for f in frame_list(row)),
            "--screenshot-dir", str(outdir), "--test-seed", "1"]
    if strict:
        cmd.append("--test-strict")
    if WINDOW:
        cmd += ["--test-window", WINDOW]
    if env is None:
        env = dict(os.environ)
    if PLATFORM == "windows":
        env["PATH"] = str(DLL_DIR) + os.pathsep + env["PATH"]
    with open(outdir / "log.txt", "w") as log:
        try:
            p = subprocess.Popen(cmd, cwd=cwd_path(row), env=env, stdout=log, stderr=subprocess.STDOUT,
                                 start_new_session=(PLATFORM == "linux"))
        except OSError as e:
            log.write(f"cannot launch: {e}\n")
            return "cannot launch"
        try:
            return p.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            # Kill the whole tree, a project may leave children behind
            if PLATFORM == "windows":
                subprocess.run(["taskkill", "/T", "/F", "/PID", str(p.pid)], capture_output=True)
            else:
                try:
                    os.killpg(p.pid, signal.SIGKILL)
                except OSError:
                    pass
            p.wait()
            return "timeout"


def read_info(outdir):
    info = outdir / "info.json"
    try:
        return json.loads(info.read_text()) if info.exists() else {}
    except ValueError:
        return {}


def cross_off_reason(row):
    # Why the other machines' goldens are not compared, from the notes of the row
    for note in (row.get(NOTE, ""), row.get("note", ""), row.get("linux_note", "")):
        if "not compared" in note:
            return note.split(", so the")[0].strip()
    return "cross_max_bad=off in the manifest"


def check(row, outdir, update, method=""):
    # Compares the captures strictly with the goldens of this machine and, with a
    # larger tolerance, with the goldens of every other machine. Returns the status,
    # the result per golden set {name: [result, detail]} and the frame names. A
    # result is pass, fail, updated, missing (no golden), n/a (the project doesn't
    # exist on that machine's platform) or off (not compared, see cross_max_bad).
    meta = read_info(outdir)
    masks = parse_masks(row["mask"])
    tol = int(row["tol"] or DEFAULT_TOL)
    max_bad = float(row["max_bad"] or DEFAULT_MAX_BAD)
    cross_tol = int(row.get("cross_tol") or DEFAULT_CROSS_TOL)
    # 'off': the image cannot match the other machines
    cross_off = row.get("cross_max_bad") == "off"
    cross_max_bad = 1.0 if cross_off else float(row.get("cross_max_bad") or DEFAULT_CROSS_MAX_BAD)
    others = [s for s in SETS if s["name"] != OWN]
    worst = {s["name"]: 0.0 for s in others}
    how = {s["name"]: "" for s in others}
    cross_fail = {}
    own_fail, frames = "", []
    for f in frame_list(row):
        name = f"f{f:04d}.png"
        frames.append(name)
        actual, golden = outdir / name, GOLD / OWN / row["name"] / name
        if not actual.exists():
            return f"FAIL (no capture for frame {f})", {OWN: ["fail", f"no capture for frame {f}"]}, frames
        if update:
            golden.parent.mkdir(parents=True, exist_ok=True)
            # The applications write uncompressed PNGs, so recompress
            Image.open(actual).convert("RGB").save(golden, optimize=True)
        elif own_fail:
            pass                    # report the first frame that failed
        elif not golden.exists():
            own_fail = f"no golden for {OWN}"
        else:
            ok, bad, diff = compare(actual, golden, tol, max_bad, masks)
            if diff is not None:
                Image.fromarray(diff).save(outdir / name.replace(".png", ".diff.png"))
            if not ok:
                own_fail = f"frame {f}: image size differs from the golden" if diff is None else \
                           f"frame {f}: {bad:.2%} of pixels differ"
        # The other machines: a larger tolerance, and a missing golden is not an error
        if cross_off:
            continue
        for s in others:
            other = GOLD / s["name"] / row["name"] / name
            if not other.exists():
                continue
            ok, bad, diff, h = compare_cross(actual, other, cross_tol, cross_max_bad, masks)
            Image.fromarray(diff).save(outdir / name.replace(".png", f".{s['name']}.xdiff.png"))
            worst[s["name"]] = max(worst[s["name"]], bad)
            how[s["name"]] = how[s["name"]] or h
            if not ok and s["name"] not in cross_fail:
                cross_fail[s["name"]] = f"frame {f}: {bad:.2%} of pixels differ"
    if update:
        golden_info = dict(meta, platform=PLATFORM, machine=OWN, created=f"{datetime.datetime.now():%Y-%m-%d %H:%M}",
                           commit=git("log", "-1", "--format=%h"))
        if method:
            golden_info["method"] = method
        (GOLD / OWN / row["name"] / "golden.json").write_text(json.dumps(golden_info, indent=1) + "\n")

    sets = {OWN: ["updated" if update else "missing" if own_fail.startswith("no golden") else
                  "fail" if own_fail else "pass", own_fail]}
    for s in others:
        n, extra = s["name"], f", {how[s['name']]}" if how[s["name"]] else ""
        if cross_off:
            sets[n] = ["off", cross_off_reason(row)]
        elif n in cross_fail:
            sets[n] = ["fail", cross_fail[n] + extra]
        elif any((GOLD / n / row["name"] / fr).exists() for fr in frames):
            sets[n] = ["pass", f"{worst[n]:.2%} differ{extra}"]
        else:
            sets[n] = ["missing" if on_platform(row, s["platform"]) else "n/a", ""]

    if own_fail:
        return f"FAIL ({own_fail})", sets, frames
    if cross_fail:
        # Only this machine's goldens decide: a difference from another machine is reported, not a failure
        text = "; ".join(f"{cross_fail[n]} from the {n} golden (other machine{', ' + how[n] if how[n] else ''})"
                         for n in cross_fail)
        return f"{'UPDATED' if update else 'PASS'}, but {text}", sets, frames
    if cross_off:
        note = f" (other goldens not compared: {cross_off_reason(row)})"
    else:
        parts = [f"{n} golden: {v[1]}" for n, v in sets.items() if n != OWN and v[0] == "pass"]
        parts += [f"no {n} golden" for n, v in sets.items() if v[0] == "missing"]
        if not parts and others:
            parts = [f"{PLATFORM} only"]
        note = f" ({'; '.join(parts)})" if parts else ""
    return ("UPDATED" if update else "PASS") + note, sets, frames


def audit(row, outdir, runs, strict, timeout, env=None):
    # The captures of consecutive runs must be byte-identical
    first, differing = {}, set()
    for _ in range(runs):
        code = run_once(row, outdir, strict, timeout, env)
        if code != 0:
            return fail_status(code)
        for f in frame_list(row):
            path = outdir / f"f{f:04d}.png"
            if not path.exists():
                return f"FAIL (no capture for frame {f})"
            data = path.read_bytes()
            if first.setdefault(f, data) != data:
                differing.add(f)
                Image.open(path).save(outdir / f"f{f:04d}.other.png")
    if differing:
        return "FAIL (not deterministic, frames " + ",".join(str(f) for f in sorted(differing)) + ")"
    return f"DETERMINISTIC ({runs} runs)"


def choose_method(row, outdir, displays, opt):
    # Linux, first golden of a project: the GPU if it runs the project correctly
    # in strict mode, else the software renderers. Returns (method, exit code);
    # the method is empty when none worked.
    candidates = [opt.capture] if opt.capture else METHODS
    tried = []
    for method in candidates:
        env, reason = displays.for_method(method)
        if env is None:
            if method == "hw" and not opt.capture:
                # Don't settle for software just because the session is locked
                return "", f"SKIP (cannot choose the capture method: {reason}. Unlock the session, or pass --capture sw)"
            tried.append(f"{method}: {reason}")
            continue
        code = run_once(row, outdir, method == "hw" or opt.strict, opt.timeout, env)
        info = read_info(outdir)
        renderer = info.get("renderer", "")
        captured = all((outdir / f"f{f:04d}.png").exists() for f in frame_list(row))
        if code == 0 and captured and method == "hw" and not is_software(renderer):
            # The window manager shrinks a window that doesn't fit the desktop, which
            # changes the image. The virtual X server is large enough for every project.
            size = (info.get("width"), info.get("height"))
            sw_env, _ = displays.for_method("sw")
            probe = outdir.parent / (outdir.name + ".size_check")
            probe_code = run_once(row, probe, False, opt.timeout, sw_env)
            if probe_code != 0:
                # Some projects need llvmpipe to report OpenGL 4.6
                probe_code = run_once(row, probe, False, opt.timeout, displays.for_method("sw-gl46")[0])
            if probe_code != 0:
                # Without the size of the window on the virtual X server, a window
                # that the window manager shrank cannot be told apart
                tried.append(f"hw: cannot check the window size, the software run failed "
                             f"({fail_status(probe_code)[6:-1]}, see {probe.name}/log.txt)")
                continue
            sw_size = (read_info(probe).get("width"), read_info(probe).get("height"))
            shutil.rmtree(probe, ignore_errors=True)
            if sw_size != size:
                tried.append(f"hw: the {sw_size[0]}x{sw_size[1]} window does not fit the desktop "
                             f"(it became {size[0]}x{size[1]})")
                continue
            # Run it again on the GPU, the software run replaced nothing in outdir
            return method, 0
        if code == 0 and captured and method != "hw":
            if tried:
                (outdir / "methods.txt").write_text("\n".join(tried + [f"{method}: used"]) + "\n")
            return method, 0
        if code == "not built":
            return "", code
        tried.append(f"{method}: " + (f"renders in software ({renderer})" if code == 0 and captured
                                      else fail_status(code)[6:-1] if code != 0 else "no capture"))
    (outdir / "methods.txt").write_text("\n".join(tried) + "\n")
    return "", "no capture method worked (" + "; ".join(tried) + ")"


# The order in which the categories appear in the summary of the run log
CATEGORIES = ["Passed", "Golden updated", "Deterministic", "Flaky (passed on retry)", "Failed comparison",
              "Failed comparison with another machine", "No golden", "Not deterministic",
              "Skipped (no hardware display)", "Crashed or exited with an error", "Timed out", "Capture failed",
              "API error (strict mode)", "No capture method", "Did not build", "Could not launch"]


def categorize(status):
    for prefix, category in (("PASS", "Passed"), ("UPDATED", "Golden updated"),
                             ("DETERMINISTIC", "Deterministic"), ("FLAKY", "Flaky (passed on retry)"),
                             ("SKIP", "Skipped (no hardware display)")):
        if status.startswith(prefix):
            return category
    for text, category in (("not built", "Did not build"), ("timeout", "Timed out"),
                           ("cannot launch", "Could not launch"), ("no golden", "No golden"),
                           ("other machine", "Failed comparison with another machine"),
                           ("pixels differ", "Failed comparison"), ("image size differs", "Failed comparison"),
                           ("not deterministic", "Not deterministic"), ("no capture method", "No capture method"),
                           ("no capture", "Capture failed"),
                           ("exit 2", "Capture failed"), ("exit 3", "API error (strict mode)")):
        if text in status:
            return category
    return "Crashed or exited with an error"


def fail_status(code):
    if code == "not built":
        return "FAIL (not built: the executable is missing or its project failed to build)"
    if isinstance(code, str):
        return f"FAIL ({code})"
    reason = EXIT_CODES.get(code, "crashed" if code < 0 or code > 255 else "")
    return f"FAIL (exit {code}{': ' + reason if reason else ''})"


def git(*args):
    try:
        return subprocess.run(["git", *args], cwd=REPO, capture_output=True, text=True).stdout.strip()
    except OSError:
        return ""


def build(sln, rows):
    # Returns a line for the run log and the names of the rows whose project failed to build
    if PLATFORM == "linux":
        # The master build of the Makefiles: libraries, then all projects in parallel
        with open(OUT / "build.log", "w") as log:
            r = subprocess.run(["make"], cwd=REPO, stdout=log, stderr=subprocess.STDOUT)
        failed_file = REPO / "build" / "master" / "failed"
        failed_dirs = set(failed_file.read_text().split()) if failed_file.exists() else set()
        failed = {row["name"] for row in rows if row.get("linux_dir") in failed_dirs}
        if r.returncode == 0:
            return "succeeded", set()
        if not failed_dirs:
            # A library failed, so no project was built
            return f"FAILED, see {(OUT / 'build.log').relative_to(REPO)}", {row["name"] for row in rows}
        return f"FAILED for {len(failed)} manifest project(s), see {(OUT / 'build.log').relative_to(REPO)}", failed
    errors = OUT / "build_errors.log"
    r = subprocess.run([find_msbuild(), sln, "-nologo", "-m", "-v:q", "-p:Configuration=Debug", "-p:Platform=x64",
                        f"-flp:LogFile={OUT / 'build.log'};Verbosity=minimal", f"-flp1:LogFile={errors};errorsonly"])
    if r.returncode == 0:
        return "succeeded", set()
    text = errors.read_text(errors="replace").lower() if errors.exists() else ""
    failed = {row["name"] for row in rows
              if row.get("project") and str(REPO / row["project"]).lower() in text}
    return f"FAILED for {len(failed)} manifest project(s), see {errors.relative_to(REPO)}", failed


# The columns of the summary per golden set. This machine's goldens count the
# outcome of each project; another machine's goldens count the comparisons.
SET_COLUMNS = ["Passed", "Failed", "Flaky", "No golden", "Not compared", "Skipped"]


def set_counts(results, name):
    counts = dict.fromkeys(SET_COLUMNS, 0)
    for r in results:
        result = r.get("sets", {}).get(name, ["not run", ""])[0]
        column = {"pass": "Passed", "updated": "Passed", "fail": "Failed", "error": "Failed",
                  "flaky": "Flaky", "missing": "No golden", "skipped": "Skipped"}.get(result, "Not compared")
        counts[column] += 1
    return counts


def set_problems(results, name):
    # (project, problem) for the projects that failed, were flaky or were skipped with this golden set
    own = name == OWN
    problems = []
    for r in results:
        result, detail = r.get("sets", {}).get(name, ["not run", ""])
        if result in ("fail", "error") or (own and result in ("flaky", "skipped")):
            problems.append((r["name"], detail or r["status"]))
    return problems


def md_table(headers, rows, right=()):
    # A Markdown table whose columns are also aligned in the plain text.
    # 'right': the indexes of the columns aligned to the right (numbers).
    rows = [[str(c).replace("|", "/") for c in row] for row in rows]
    widths = [max(len(h), *(len(row[i]) for row in rows)) for i, h in enumerate(headers)]

    def line(cells):
        return "| " + " | ".join(c.rjust(w) if i in right else c.ljust(w)
                                 for i, (c, w) in enumerate(zip(cells, widths))) + " |"

    rule = "|" + "|".join("-" * (w + 1) + ":" if i in right else "-" * (w + 2) for i, w in enumerate(widths)) + "|"
    return [line(headers), rule] + [line(row) for row in rows]


def split_difference(detail):
    # "frame 10: 19.19% of pixels differ, cropped: ..." -> ("10", "19.19%", "cropped: ...")
    m = re.match(r"frame (\d+): ([\d.]+%) of pixels differ(?:, (.*))?$", detail)
    return (m.group(1), m.group(2), m.group(3) or "") if m else ("", "", detail)


def summary_lines(started, mode, how_found, rows, selected, results):
    # A short Markdown summary: the result on this machine, then how the captures
    # compare with the goldens of each other machine (which doesn't decide the result)
    modified = [l for l in git("status", "--porcelain", "--untracked-files=no", "--ignore-submodules").splitlines() if l]
    failed = any(r["status"].startswith("FAIL") for r in results)
    own = set_counts(results, OWN)
    own_text = ", ".join([f"{own['Passed']} passed", f"{own['Failed']} failed"] +
                         [f"{own[c]} {c.lower()}" for c in ("Flaky", "No golden", "Skipped") if own[c]])
    commit = f"`{git('log', '-1', '--format=%h') or 'unknown'}` {git('log', '-1', '--format=%s')}" + \
             (f" (+{len(modified)} uncommitted)" if modified else "")

    lines = [f"# Regression results: {OWN}", "",
             f"Testing executed on {MACHINE.get('description') or OWN} (`{OWN}`), commit {commit}"
             + ("" if mode == "compare with goldens" else f", {mode}") + ".", "",
             f"**Result: {'FAIL' if failed else 'PASS'}**: {len(results)} run, {own_text}.", ""]
    others = [s for s in SETS if s["name"] != OWN]
    if others:
        lines.append("Other machines (larger tolerance, not part of the result):")
        for s in others:
            c = set_counts(results, s["name"])
            parts = [f"{c['Passed']} match", f"{c['Failed']} differ"] + \
                    [f"{c[k]} {k.lower()}" for k in ("No golden", "Not compared") if c[k]]
            lines.append(f"- `{s['name']}`" + (f", {s['description']}" if s["description"] else "") + ": " + ", ".join(parts))
        lines.append("")

    problems = set_problems(results, OWN)
    if problems:
        lines += ["## Problems on this machine", ""] + md_table(["Project", "Problem"], problems) + [""]
    differ = [(p, s["name"], *split_difference(d)) for s in others for p, d in set_problems(results, s["name"])]
    if differ:
        lines += ["## Differences from the other machines", ""]
        lines += md_table(["Project", "Golden set", "Frame", "Pixels differ", "Note"], differ, right=(2, 3)) + [""]
    return lines


def write_summary(lines):
    RESULTS.mkdir(exist_ok=True)
    path = RESULTS / f"{OWN}.md"
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return path


def last_log(log):
    # A copy of the run log under a fixed name, which is what the terminal shows:
    # the name of the run log has the date and time, and the terminal output is
    # meant to be pasted elsewhere
    path = OUT / "last_run.log"
    shutil.copyfile(log, path)
    return path


def write_run_log(started, opt, mode, build_line, rows, selected, results, missing, how_found):
    finished = datetime.datetime.now()
    seconds = int((finished - started).total_seconds())
    modified = [l for l in git("status", "--porcelain", "--untracked-files=no", "--ignore-submodules").splitlines() if l]
    commit = git("log", "-1", "--format=%h  %s") or "unknown"
    gpus = sorted({r["renderer"] for r in results if r.get("renderer")})
    methods = {}
    for r in results:
        if r.get("method"):
            methods[r["method"]] = methods.get(r["method"], 0) + 1
    width = max([len(r["name"]) for r in rows] + [10])
    counts = {}
    for r in results:
        category = categorize(r["status"])
        counts[category] = counts.get(category, 0) + 1
    bad = [r for r in results if r["status"].startswith(("FAIL", "FLAKY", "SKIP")) or "but" in r["status"]]
    sep = "/" if PLATFORM == "linux" else "\\"

    lines = ["OGLDEV regression run", "=====================", "",
             f"Date:        {started:%Y-%m-%d}",
             f"Time:        {started:%H:%M:%S}  (took {seconds // 60}m {seconds % 60:02d}s)",
             f"Git commit:  {commit}",
             f"Git branch:  {git('rev-parse', '--abbrev-ref', 'HEAD') or 'unknown'}"
             + (f"  (+ {len(modified)} uncommitted modified file(s))" if modified else ""),
             f"Machine:     {OWN} ({MACHINE.get('description') or '-'}, detected by {how_found}), host {socket.gethostname()}",
             f"Platform:    {PLATFORM}",
             f"Goldens:     {', '.join(s['name'] for s in SETS)}",
             f"GPU:         {', '.join(gpus) or 'unknown'}"]
    if methods:
        lines.append("Capture:     " + ", ".join(f"{m} {n}" for m, n in sorted(methods.items())))
    lines += [f"Mode:        {mode}",
              f"Build:       {build_line}",
              f"Command:     {' '.join(['regress.py'] + sys.argv[1:])}", ""]
    if missing:
        lines += manifest_warning(missing) + [""]

    lines += ["Summary", "-------", f"{'Projects run':42}{len(results):5}"]
    lines += [f"  {c:40}{counts[c]:5}" for c in CATEGORIES if c in counts]
    not_run = [r for r in rows if r not in selected]
    skipped = [r for r in not_run if r[STATUS] == "skip"]
    todo = [r for r in not_run if r[STATUS] == "todo"]
    lines += [f"{'Projects not run':42}{len(not_run):5}",
              f"  {'Excluded in the manifest':40}{len(skipped):5}",
              f"  {'Not enabled yet (todo)':40}{len(todo):5}",
              f"  {'Not selected for this run':40}{len(not_run) - len(skipped) - len(todo):5}",
              f"{'Projects missing from the manifest':42}{len(missing):5}", "",
              "Result:      " + ("FAILED" if any(r["status"].startswith("FAIL") for r in results) else "OK"), ""]
    if any(r.get("sets") for r in results):
        lines += ["Per golden set", "--------------"]
        lines += [f"{s['name']:20}" + "  ".join(f"{c} {n}" for c, n in set_counts(results, s["name"]).items()
                                                if n and (s["name"] == OWN or c not in ("Flaky", "Skipped")))
                  for s in SETS] + [""]

    if bad:
        lines += ["Problems", "--------"]
        lines += [f"{r['name']:{width}}  {r['status']}" for r in bad]
        lines += ["", f"Each project's own output is in {OUT.relative_to(REPO)}{sep}<project>{sep}log.txt", ""]

    lines += ["All projects that ran", "---------------------"]
    lines += [f"{r['name']:{width}}  {r['status']}" + (f"  [{r['method']}]" if r.get("method") else "")
              for r in results] or ["(none)"]
    lines += ["", "Excluded in the manifest", "------------------------"]
    lines += [f"{r['name']:{width}}  {r[NOTE]}" for r in skipped] or ["(none)"]
    lines += ["", "Not enabled yet (todo)", "----------------------"]
    lines += [f"{r['name']:{width}}  {r[NOTE]}" for r in todo] or ["(none)"]

    path = OUT / f"run_{started:%Y-%m-%d_%H-%M-%S}.log"
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return path


def set_status(rows, enable, disable):
    # --enable / --disable: switch manifest rows between 'todo' and 'ok' on this platform, then exit
    by_name = {r["name"]: r for r in rows}
    unknown = [n for n in enable + disable if n not in by_name]
    if unknown:
        sys.exit(f"not in the manifest: {', '.join(unknown)}")
    elsewhere = [n for n in enable + disable if not on_platform(by_name[n])]
    if elsewhere:
        sys.exit(f"not a {PLATFORM} project: {', '.join(elsewhere)}")
    both = set(enable) & set(disable)
    if both:
        sys.exit(f"both enabled and disabled: {', '.join(sorted(both))}")
    changed, refused = 0, 0
    for names, status in ((enable, "ok"), (disable, "todo")):
        for name in names:
            row = by_name[name]
            if row[STATUS] == "skip":
                refused += 1
                print(f"{name:32} not changed: excluded in the manifest ({row[NOTE]})")
            elif row[STATUS] == status:
                print(f"{name:32} already '{status}'")
            else:
                print(f"{name:32} {row[STATUS] or '-'} -> {status}")
                row[STATUS] = status
                changed += 1
            if status == "ok" and row[STATUS] == "ok" and not any((GOLD / OWN / name).glob("f*.png")):
                print(f"{'':32} note: it has no {OWN} golden yet (--update-golden --only {name})")
    if changed:
        write_manifest(rows)
    print(f"{changed} row(s) changed in {MANIFEST.relative_to(REPO)}")
    return 1 if refused else 0


def report_section(status):
    if status.startswith("FAIL"):
        return "Failed"
    if status.startswith("FLAKY"):
        return "Flaky"
    if status.startswith("SKIP"):
        return "Skipped"
    return "Passed"


def write_report(results):
    # One table per outcome: the failures first, then flaky, skipped and passed
    others = [s["name"] for s in SETS if s["name"] != OWN]
    header = (f"<table border=1 cellpadding=4><tr><th>frame<th>status<th>{OWN} golden<th>actual<th>difference x8"
              + "".join(f"<th>{html.escape(o)} golden<th>difference x8" for o in others))
    with open(OUT / "report.html", "w") as f:
        f.write(f"<meta charset=utf-8><title>Regression report</title><h1>Regression report: {html.escape(OWN)}</h1>")
        for section in ("Failed", "Flaky", "Skipped", "Passed"):
            group = [r for r in results if report_section(r["status"]) == section]
            if group:
                f.write(f"<h2>{section} ({len(group)})</h2>" + header)
                write_report_rows(f, group, others)
                f.write("</table>")


def write_report_rows(f, results, others):
    for r in results:
        for name in r["frames"] or [""]:
            base = f"{r['name']}/{name}"
            f.write(f"<tr><td>{html.escape(base)}<td>{html.escape(r['status'])}")
            if not name:
                f.write(f"<td colspan={3 + 2 * len(others)}><a href='{html.escape(r['name'])}/log.txt'>log</a>")
                continue
            f.write(f"<td><img src='../goldens/{OWN}/{html.escape(base)}' width=260>"
                    f"<td><img src='{html.escape(base)}' width=260>"
                    f"<td><img src='{html.escape(base.replace('.png', '.diff.png'))}' width=260>")
            for o in others:
                result = r.get("sets", {}).get(o, ["not run", ""])[0]
                if result in ("pass", "fail"):
                    f.write(f"<td><img src='../goldens/{html.escape(o)}/{html.escape(base)}' width=260>"
                            f"<td><img src='{html.escape(base.replace('.png', f'.{o}.xdiff.png'))}' width=260>")
                else:
                    f.write(f"<td colspan=2>{html.escape(result)}")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--only", nargs="*", help="project names; also runs rows whose status is still 'todo'")
    ap.add_argument("--api")
    ap.add_argument("--tier")
    ap.add_argument("--update-golden", action="store_true")
    ap.add_argument("--retries", type=int, default=1)
    ap.add_argument("--strict", action="store_true")
    ap.add_argument("--machine", default=os.environ.get("OGLDEV_MACHINE"),
                    help="the name of this machine in Testing/machines.csv (default: detected)")
    ap.add_argument("--timeout", type=int, default=DEFAULT_TIMEOUT)
    ap.add_argument("--build", metavar="SLN", nargs="?", const=str(DEFAULT_SLN))
    ap.add_argument("--coverage", action="store_true")
    ap.add_argument("--audit", type=int, metavar="RUNS", help="run each project RUNS times and compare the captures byte for byte")
    ap.add_argument("--enable", nargs="+", default=[], metavar="NAME", help="set the status of these projects to 'ok' on this platform and exit")
    ap.add_argument("--disable", nargs="+", default=[], metavar="NAME", help="set the status of these projects back to 'todo' on this platform and exit")
    ap.add_argument("--capture", choices=METHODS,
                    help="Linux: with --update-golden, use (and record) this capture method; with --audit, use it")
    ap.add_argument("--window", default="native", metavar="WxH|native",
                    help="size of the OpenGL windows in test mode (default: the size each project asks for)")
    opt = ap.parse_args()
    global WINDOW
    WINDOW = "" if opt.window == "native" else opt.window
    if WINDOW and not re.fullmatch(r"[1-9]\d*x[1-9]\d*", WINDOW):
        sys.exit("--window takes WIDTHxHEIGHT or 'native'")
    if opt.capture and not (opt.update_golden or opt.audit):
        sys.exit("--capture is only used with --update-golden or --audit")

    global MACHINE, OWN, SETS
    machines = read_machines()
    MACHINE, how_found = detect_machine(machines, opt.machine)
    OWN = MACHINE["name"]
    SETS = golden_sets(machines)
    print(f"machine: {OWN} ({MACHINE.get('description') or '-'}, detected by {how_found})", flush=True)

    with open(MANIFEST, newline="") as f:
        all_rows = list(csv.DictReader(f))
    if opt.enable or opt.disable:
        return set_status(all_rows, opt.enable, opt.disable)
    # The rows that have a project on this platform
    rows = [r for r in all_rows if on_platform(r)]
    missing = missing_from_manifest(all_rows, opt.build or DEFAULT_SLN)
    for line in manifest_warning(missing):
        print(line, flush=True)
    if opt.coverage:
        for key in (STATUS, "tier", "api") + (("linux_capture",) if PLATFORM == "linux" else ()):
            counts = {}
            for r in rows:
                counts[r[key] or "-"] = counts.get(r[key] or "-", 0) + 1
            print(key, counts)
        return 0
    if opt.only:
        unknown = set(opt.only) - {r["name"] for r in rows}
        if unknown:
            sys.exit(f"not a {PLATFORM} project in the manifest: {', '.join(sorted(unknown))}")
    started = datetime.datetime.now()
    OUT.mkdir(exist_ok=True)
    build_line, not_built = "not requested (existing executables were used)", set()
    if opt.build:
        build_line, not_built = build(opt.build, rows)
    selected = [r for r in rows
                if (r["name"] in opt.only and r[STATUS] != "skip" if opt.only else r[STATUS] == "ok")
                and (not opt.api or r["api"] == opt.api)
                and (not opt.tier or r["tier"] == opt.tier)]
    displays = LinuxDisplays(opt.timeout) if PLATFORM == "linux" else None
    if displays:
        # Restore the screen saver settings and stop Xvfb also when the run is killed
        for sig in (signal.SIGTERM, signal.SIGHUP):
            signal.signal(sig, lambda *_: sys.exit(1))
    results, failed = [], 0

    def recorded_method(r):
        # Linux: the capture method of this machine's golden of the project
        try:
            return json.loads((GOLD / OWN / r["name"] / "golden.json").read_text()).get("method", "")
        except (OSError, ValueError):
            return ""

    def environment(r, outdir):
        # Linux: the environment of the row's recorded capture method.
        # Returns (env, method, status) where status is set when the row cannot run.
        if PLATFORM != "linux":
            return None, "", None
        method = recorded_method(r)
        if opt.update_golden and (opt.capture or not method):
            return None, "", None          # choose_method() decides
        if not method:
            return None, "", f"FAIL (no golden for {OWN}: create it with --update-golden)"
        env, reason = displays.for_method(method)
        if env is None:
            return None, method, f"SKIP ({method} capture: {reason})"
        return env, method, None

    try:
        if opt.audit:
            for r in selected:
                env, method, status = None, "", None
                if PLATFORM == "linux":
                    # A new project has no recorded method yet: audit it in software unless told otherwise
                    method = opt.capture or recorded_method(r) or "sw"
                    env, reason = displays.for_method(method)
                    if env is None:
                        status = f"SKIP ({method} capture: {reason})"
                if r["name"] in not_built:
                    status = fail_status("not built")
                elif status is None:
                    status = audit(r, OUT / r["name"], opt.audit, opt.strict, opt.timeout, env)
                failed += status.startswith("FAIL")
                results.append({"name": r["name"], "status": status, "method": method})
                print(f"{r['name']:32} {status}", flush=True)
            print(f"{len(selected)} audited, {failed} failed")
            log = write_run_log(started, opt, f"determinism audit, {opt.audit} runs per project", build_line, rows,
                                selected, results, missing, how_found)
            print(f"log: {last_log(log).relative_to(REPO)}")
            return 1 if failed else 0

        for r in selected:
            outdir, attempt = OUT / r["name"], 0
            env, method, status = environment(r, outdir)
            frames, sets = [], {}
            while status is None:
                attempt += 1
                if r["name"] in not_built:
                    code = "not built"
                elif PLATFORM == "linux" and env is None:
                    # First golden on this Linux machine: find the capture method
                    # (check() records it in the golden.json)
                    method, code = choose_method(r, outdir, displays, opt)
                    if isinstance(code, str) and code.startswith("SKIP"):
                        status = code
                        break
                    if method:
                        env, _ = displays.for_method(method)
                else:
                    code = run_once(r, outdir, opt.strict, opt.timeout, env)
                if code != 0:
                    status, frames = fail_status(code), []
                    sets = {OWN: ["error", status[6:-1]]}
                else:
                    status, sets, frames = check(r, outdir, opt.update_golden, method)
                if not status.startswith("FAIL") or attempt > opt.retries or opt.update_golden or code == "not built":
                    break
                status = None
            if status.startswith("SKIP"):
                sets = {OWN: ["skipped", status[6:-1]]}
            elif not sets:
                # environment() refused to run it: no golden on this Linux machine
                sets = {OWN: ["missing" if "no golden" in status else "error", status[6:-1]]}
            if attempt > 1 and not status.startswith("FAIL"):
                status = "FLAKY (passed on retry)"
                sets[OWN] = ["flaky", f"passed on attempt {attempt}"]
            failed += status.startswith("FAIL")
            results.append({"name": r["name"], "status": status, "gpu": OWN, "frames": frames, "sets": sets,
                            "method": method, "cross_off": r.get("cross_max_bad") == "off", "renderer": read_info(outdir).get("renderer", "")})
            print(f"{r['name']:32} {status}" + (f"  [{method}]" if method else ""), flush=True)
    finally:
        if displays:
            displays.close()

    write_report(results)
    (OUT / "results.json").write_text(json.dumps(results, indent=1))
    flaky = sum(r["status"].startswith("FLAKY") for r in results)
    skipped_hw = sum(r["status"].startswith("SKIP") for r in results)
    print(f"{len(results)} run, {failed} failed, {flaky} flaky" + (f", {skipped_hw} skipped (no hardware display)" if skipped_hw else ""))
    if skipped_hw and displays:
        print(f"hardware display: {displays.hw_reason}")
    mode = ("update goldens" if opt.update_golden else "compare with goldens") + (", strict" if opt.strict else "")
    log = write_run_log(started, opt, mode, build_line, rows, selected, results, missing, how_found)
    summary = summary_lines(started, mode, how_found, rows, selected, results)
    with open(log, "a", encoding="utf-8") as f:
        f.write("\n" + "\n".join(summary) + "\n")
    path = write_summary(summary)
    print("\n" + "\n".join(summary))
    print(f"log: {last_log(log).relative_to(REPO)}")
    print(f"summary: {path.relative_to(REPO)}")
    return 1 if failed else 0


sys.exit(main())
