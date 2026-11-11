# Golden-image regression runner, for Windows and Linux.
#
#   python Testing/regress.py                       run every 'ok' row of Testing/manifest.csv
#   python Testing/regress.py --only Name1 Name2     run a subset (also rows that are still 'todo')
#   python Testing/regress.py --update-golden --only Name1
#   python Testing/regress.py --coverage
#
# Each project is launched in its test mode (see Include/regress_hook.h), the
# frames it captured are compared with the goldens of this platform,
# Testing/goldens/<platform>/<project>/fNNNN.png, and with the goldens of the
# other platform using a larger tolerance. The result goes to
# Testing/out/report.html and Testing/out/results.json, and every run leaves a
# readable Testing/out/run_<date>_<time>.log with the status of each project.
#
# On Linux a project is captured either on the GPU of the desktop session
# ('hw') or with Mesa's software renderers on a virtual X server ('sw').
# The method is chosen when the golden is first created and recorded in the
# manifest (linux_capture), and later runs always use it.
#
# Needs: pip install pillow numpy (on Linux: apt install python3-pil python3-numpy xvfb mesa-utils)

import argparse, atexit, csv, datetime, html, json, os, re, shutil, signal, socket, subprocess, sys, time
from pathlib import Path
import numpy as np
from PIL import Image

REPO = Path(__file__).resolve().parent.parent
TESTING = REPO / "Testing"
MANIFEST = TESTING / "manifest.csv"
OUT, GOLD = TESTING / "out", TESTING / "goldens"
DLL_DIR = REPO / "Windows" / "DLL"
DEFAULT_SLN = REPO / "Windows" / "ogldev_vs_2026" / "ogldev_vs_2026.sln"
DEFAULT_TOL, DEFAULT_MAX_BAD, DEFAULT_TIMEOUT = 8, 0.001, 120
# Comparison with the goldens of the other platform (different GPU, driver and OS)
DEFAULT_CROSS_TOL, DEFAULT_CROSS_MAX_BAD = 48, 0.02
EXIT_CODES = {2: "capture failed", 3: "API error in strict mode"}

PLATFORM = "linux" if sys.platform.startswith("linux") else "windows"
# --window: the size of the OpenGL windows in test mode (--test-window). By default
# every project keeps the size it asks for, because many tutorials size their
# viewport and projection from their own constants and draw only part of the
# image into a smaller window. The comparison with the other platform scales
# the larger image down.
WINDOW = ""
OTHER = "windows" if PLATFORM == "linux" else "linux"
STATUS = "linux_status" if PLATFORM == "linux" else "status"
NOTE = "linux_note" if PLATFORM == "linux" else "note"

# Linux capture methods, in the order they are tried when a golden is first created
#   hw      the GPU, on the X display of the desktop session
#   sw      Mesa llvmpipe (OpenGL) and lavapipe (Vulkan) on a virtual X server
#   sw-gl46 the same, with llvmpipe reporting OpenGL 4.6 (it implements 4.5)
METHODS = ["hw", "sw", "sw-gl46"]
SOFTWARE_RENDERERS = ("llvmpipe", "softpipe", "lavapipe", "swrast", "software")
LAVAPIPE_ICD = "/usr/share/vulkan/icd.d/lvp_icd.json"


# ---------------------------------------------------------------------------
#  Platform specific details of a manifest row
# ---------------------------------------------------------------------------

def on_platform(row):
    return bool(row.get("linux_dir")) if PLATFORM == "linux" else bool(row.get("exe"))


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


def on_other_platform(row):
    return bool(row.get("exe")) if PLATFORM == "linux" else bool(row.get("linux_dir"))


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
#  Linux displays: the desktop session for 'hw', a private Xvfb for 'sw'
# ---------------------------------------------------------------------------

class LinuxDisplays:
    def __init__(self, timeout):
        self.timeout = timeout
        self.xvfb, self.sw_display = None, None
        self.hw_checked, self.hw_display, self.hw_renderer, self.hw_reason = False, None, "", ""
        self.xset_restore = None
        atexit.register(self.close)

    def software(self):
        if self.xvfb is None:
            if not shutil.which("Xvfb"):
                sys.exit("Xvfb is not installed (sudo apt install xvfb)")
            for n in range(90, 110):
                if not Path(f"/tmp/.X11-unix/X{n}").exists() and not Path(f"/tmp/.X{n}-lock").exists():
                    break
            # Large enough for the projects that ask for big windows
            self.xvfb = subprocess.Popen(["Xvfb", f":{n}", "-screen", "0", "2560x1600x24", "-nolisten", "tcp"],
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
        return display

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
    # The comparison with the other platform's golden, which may have another size.
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


def check(row, outdir, golden_set, update, method=""):
    # Compares the captures with the goldens of this platform and of the other one.
    # Returns the status, the golden folder of this platform and the frame names.
    meta = read_info(outdir)
    own = golden_set or PLATFORM
    masks = parse_masks(row["mask"])
    tol = int(row["tol"] or DEFAULT_TOL)
    max_bad = float(row["max_bad"] or DEFAULT_MAX_BAD)
    cross_tol = int(row.get("cross_tol") or DEFAULT_CROSS_TOL)
    # 'off': the image cannot match the other platform (e.g. it depends on rand())
    cross_off = row.get("cross_max_bad") == "off"
    cross_max_bad = 1.0 if cross_off else float(row.get("cross_max_bad") or DEFAULT_CROSS_MAX_BAD)
    frames, cross_worst, cross_missing, cross_fail, cross_how = [], 0.0, False, None, ""
    for f in frame_list(row):
        name = f"f{f:04d}.png"
        frames.append(name)
        actual, golden, other = outdir / name, GOLD / own / row["name"] / name, GOLD / OTHER / row["name"] / name
        if not actual.exists():
            return f"FAIL (no capture for frame {f})", own, frames
        if update:
            golden.parent.mkdir(parents=True, exist_ok=True)
            # The applications write uncompressed PNGs, so recompress
            Image.open(actual).convert("RGB").save(golden, optimize=True)
        elif not golden.exists():
            return f"FAIL (no golden for {own})", own, frames
        else:
            ok, bad, diff = compare(actual, golden, tol, max_bad, masks)
            if diff is not None:
                Image.fromarray(diff).save(outdir / name.replace(".png", ".diff.png"))
            if not ok:
                if diff is None:
                    return f"FAIL (frame {f}: image size differs from the golden)", own, frames
                return f"FAIL (frame {f}: {bad:.2%} of pixels differ)", own, frames
        # The other platform: a larger tolerance, and a missing golden is not an error
        if cross_off:
            continue
        if not other.exists():
            cross_missing = True
            continue
        ok, bad, diff, how = compare_cross(actual, other, cross_tol, cross_max_bad, masks)
        Image.fromarray(diff).save(outdir / name.replace(".png", ".xdiff.png"))
        cross_worst = max(cross_worst, bad)
        cross_how = cross_how or how
        if not ok and cross_fail is None:
            cross_fail = f"frame {f}: {bad:.2%} of pixels differ from"
    if update:
        golden_info = dict(meta, platform=PLATFORM, created=f"{datetime.datetime.now():%Y-%m-%d %H:%M}",
                           commit=git("log", "-1", "--format=%h"))
        if method:
            golden_info["method"] = method
        (GOLD / own / row["name"] / "golden.json").write_text(json.dumps(golden_info, indent=1) + "\n")
    if cross_fail:
        text = f"{cross_fail} the {OTHER} golden (other platform{', ' + cross_how if cross_how else ''})"
        return (f"UPDATED, but {text}" if update else f"FAIL ({text})"), own, frames
    note = f" ({OTHER} golden not compared)" if cross_off else \
        f" ({PLATFORM} only)" if cross_missing and not on_other_platform(row) else \
        f" (no {OTHER} golden)" if cross_missing else \
        f" ({OTHER} golden: {cross_worst:.2%} differ{', ' + cross_how if cross_how else ''})"
    return ("UPDATED" if update else "PASS") + note, own, frames


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
              "Failed comparison with the other platform", "No golden", "Not deterministic",
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
                           ("other platform", "Failed comparison with the other platform"),
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


def write_run_log(started, opt, mode, build_line, rows, selected, results, missing):
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
             f"Machine:     {socket.gethostname()}",
             f"Platform:    {PLATFORM}",
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
            if status == "ok" and row[STATUS] == "ok" and not any((GOLD / PLATFORM / name).glob("f*.png")):
                print(f"{'':32} note: it has no {PLATFORM} golden yet (--update-golden --only {name})")
    if changed:
        write_manifest(rows)
    print(f"{changed} row(s) changed in {MANIFEST.relative_to(REPO)}")
    return 1 if refused else 0


def write_report(results):
    with open(OUT / "report.html", "w") as f:
        f.write("<meta charset=utf-8><title>Regression report</title>"
                "<table border=1 cellpadding=4><tr><th>frame<th>status<th>golden<th>actual<th>difference x8"
                f"<th>{OTHER} golden<th>difference x8")
        for r in results:
            for name in r["frames"] or [""]:
                base = f"{r['name']}/{name}"
                f.write(f"<tr><td>{html.escape(base)}<td>{html.escape(r['status'])}")
                if not name:
                    f.write(f"<td colspan=5><a href='{html.escape(r['name'])}/log.txt'>log</a>")
                    continue
                f.write(f"<td><img src='../goldens/{r['gpu']}/{html.escape(base)}' width=260>"
                        f"<td><img src='{html.escape(base)}' width=260>"
                        f"<td><img src='{html.escape(base.replace('.png', '.diff.png'))}' width=260>"
                        f"<td><img src='../goldens/{OTHER}/{html.escape(base)}' width=260>"
                        f"<td><img src='{html.escape(base.replace('.png', '.xdiff.png'))}' width=260>")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--only", nargs="*", help="project names; also runs rows whose status is still 'todo'")
    ap.add_argument("--api")
    ap.add_argument("--tier")
    ap.add_argument("--update-golden", action="store_true")
    ap.add_argument("--retries", type=int, default=1)
    ap.add_argument("--strict", action="store_true")
    ap.add_argument("--gpu", help="use the goldens in Testing/goldens/GPU instead of Testing/goldens/<platform>")
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
    results, failed, manifest_changed = [], 0, False

    def environment(r, outdir):
        # Linux: the environment of the row's recorded capture method.
        # Returns (env, method, status) where status is set when the row cannot run.
        if PLATFORM != "linux":
            return None, "", None
        method = r.get("linux_capture", "")
        if opt.update_golden and (opt.capture or not method):
            return None, "", None          # choose_method() decides
        if not method:
            return None, "", f"FAIL (no golden for {PLATFORM}: create it with --update-golden)"
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
                    method = opt.capture or r.get("linux_capture") or "sw"
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
            log = write_run_log(started, opt, f"determinism audit, {opt.audit} runs per project", build_line, rows, selected, results, missing)
            print(f"log: {log.relative_to(REPO)}")
            return 1 if failed else 0

        for r in selected:
            outdir, attempt = OUT / r["name"], 0
            env, method, status = environment(r, outdir)
            gpu, frames = opt.gpu or PLATFORM, []
            while status is None:
                attempt += 1
                if r["name"] in not_built:
                    code = "not built"
                elif PLATFORM == "linux" and env is None:
                    # First golden on Linux: find the capture method and keep it
                    method, code = choose_method(r, outdir, displays, opt)
                    if isinstance(code, str) and code.startswith("SKIP"):
                        status = code
                        break
                    if method and method != r.get("linux_capture"):
                        r["linux_capture"] = method
                        manifest_changed = True
                    if method:
                        env, _ = displays.for_method(method)
                else:
                    code = run_once(r, outdir, opt.strict, opt.timeout, env)
                if code != 0:
                    status = fail_status(code)
                else:
                    status, gpu, frames = check(r, outdir, opt.gpu, opt.update_golden, method)
                if not status.startswith("FAIL") or attempt > opt.retries or opt.update_golden or code == "not built":
                    break
                status = None
            if attempt > 1 and not status.startswith("FAIL"):
                status = "FLAKY (passed on retry)"
            failed += status.startswith("FAIL")
            results.append({"name": r["name"], "status": status, "gpu": gpu, "frames": frames,
                            "method": method, "renderer": read_info(outdir).get("renderer", "")})
            print(f"{r['name']:32} {status}" + (f"  [{method}]" if method else ""), flush=True)
    finally:
        if manifest_changed:
            write_manifest(all_rows)
            print(f"recorded the capture method of the new goldens in {MANIFEST.relative_to(REPO)}")
        if displays:
            displays.close()

    write_report(results)
    (OUT / "results.json").write_text(json.dumps(results, indent=1))
    flaky = sum(r["status"].startswith("FLAKY") for r in results)
    skipped_hw = sum(r["status"].startswith("SKIP") for r in results)
    print(f"{len(results)} run, {failed} failed, {flaky} flaky" + (f", {skipped_hw} skipped (no hardware display)" if skipped_hw else ""))
    if skipped_hw and displays:
        print(f"hardware display: {displays.hw_reason}")
    mode = "update goldens" if opt.update_golden else "compare with goldens"
    log = write_run_log(started, opt, mode + (", strict" if opt.strict else ""), build_line, rows, selected, results, missing)
    print(f"log: {log.relative_to(REPO)}")
    return 1 if failed else 0


sys.exit(main())
