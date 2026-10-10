# Golden-image regression runner.
#
#   python Testing/regress.py                       run every 'ok' row of Testing/manifest.csv
#   python Testing/regress.py --only Name1 Name2     run a subset (also rows that are still 'todo')
#   python Testing/regress.py --update-golden --only Name1
#   python Testing/regress.py --coverage
#
# Each project is launched in its test mode (see Include/regress_hook.h), the
# frames it captured are compared with Testing/goldens/<gpu>/<project>/fNNNN.png
# and the result goes to Testing/out/report.html and Testing/out/results.json.
# Goldens are local to a machine and are not committed. Every run also leaves a
# readable Testing/out/run_<date>_<time>.log with the status of each project.
#
# Needs: pip install pillow numpy

import argparse, csv, datetime, html, json, os, re, shutil, socket, subprocess, sys, time
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
EXIT_CODES = {2: "capture failed", 3: "API error in strict mode"}


def find_msbuild():
    if os.environ.get("MSBUILD"):
        return os.environ["MSBUILD"]
    vswhere = Path(os.environ["ProgramFiles(x86)"]) / "Microsoft Visual Studio" / "Installer" / "vswhere.exe"
    r = subprocess.run([str(vswhere), "-latest", "-prerelease", "-requires", "Microsoft.Component.MSBuild",
                        "-find", r"MSBuild\**\Bin\MSBuild.exe"], capture_output=True, text=True)
    lines = r.stdout.split("\n")
    return lines[0].strip() if lines[0].strip() else "msbuild"


def missing_from_manifest(rows, sln=DEFAULT_SLN):
    # Application projects of the solution that have no manifest row, so they are never tested
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
    return [f"WARNING: {len(missing)} project(s) of the solution are missing from the manifest and are not tested:"] + \
           [f"  {m}" for m in missing] + \
           ["  To add them: python Testing\\make_manifest.py Windows\\ogldev_vs_2026\\ogldev_vs_2026.sln",
            "  then try each one with --only and set its status to 'ok' (see Testing\\README.md)."]


def slug(s):
    return re.sub(r"[^a-z0-9]+", "-", s.lower()).strip("-")[:60] or "unknown"


def parse_masks(text):
    return [tuple(int(v) for v in m.split(",")) for m in text.split(";") if m.strip()]


def frame_list(row):
    return [int(f) for f in (row["frames"] or "10").split(";") if f.strip()]


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


def run_once(row, outdir, strict, timeout):
    if not (REPO / row["exe"]).exists():
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
    cmd = [str(REPO / row["exe"])] + row["args"].split()
    cmd += ["--test-frames", ",".join(str(f) for f in frame_list(row)),
            "--screenshot-dir", str(outdir), "--test-seed", "1"]
    if strict:
        cmd.append("--test-strict")
    env = dict(os.environ)
    env["PATH"] = str(DLL_DIR) + os.pathsep + env["PATH"]
    with open(outdir / "log.txt", "w") as log:
        try:
            p = subprocess.Popen(cmd, cwd=REPO / row["cwd"], env=env, stdout=log, stderr=subprocess.STDOUT)
        except OSError as e:
            log.write(f"cannot launch: {e}\n")
            return "cannot launch"
        try:
            return p.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            # Kill the whole tree, a project may leave children behind
            subprocess.run(["taskkill", "/T", "/F", "/PID", str(p.pid)], capture_output=True)
            p.wait()
            return "timeout"


def check(row, outdir, gpu_override, update):
    info = outdir / "info.json"
    meta = json.loads(info.read_text()) if info.exists() else {}
    # OpenGL reports "<name>/PCIe/SSE2" and Vulkan just "<name>": keep one folder per GPU
    gpu = gpu_override or slug(meta.get("renderer", "unknown").split("/")[0])
    masks = parse_masks(row["mask"])
    tol = int(row["tol"] or DEFAULT_TOL)
    max_bad = float(row["max_bad"] or DEFAULT_MAX_BAD)
    frames = []
    for f in frame_list(row):
        name = f"f{f:04d}.png"
        frames.append(name)
        actual, golden = outdir / name, GOLD / gpu / row["name"] / name
        if not actual.exists():
            return f"FAIL (no capture for frame {f})", gpu, frames
        if update:
            golden.parent.mkdir(parents=True, exist_ok=True)
            # The applications write uncompressed PNGs, so recompress
            Image.open(actual).convert("RGB").save(golden, optimize=True)
            continue
        if not golden.exists():
            return f"FAIL (no golden for {gpu})", gpu, frames
        ok, bad, diff = compare(actual, golden, tol, max_bad, masks)
        if diff is not None:
            Image.fromarray(diff).save(outdir / name.replace(".png", ".diff.png"))
        if not ok:
            if diff is None:
                return f"FAIL (frame {f}: image size differs from the golden)", gpu, frames
            return f"FAIL (frame {f}: {bad:.2%} of pixels differ)", gpu, frames
    return ("UPDATED" if update else "PASS"), gpu, frames


def audit(row, outdir, runs, strict, timeout):
    # Step 4.1 of the plan: the captures of consecutive runs must be byte-identical
    first, differing = {}, set()
    for _ in range(runs):
        code = run_once(row, outdir, strict, timeout)
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


# The order in which the categories appear in the summary of the run log
CATEGORIES = ["Passed", "Golden updated", "Deterministic", "Flaky (passed on retry)", "Failed comparison",
              "No golden", "Not deterministic", "Crashed or exited with an error", "Timed out", "Capture failed",
              "API error (strict mode)", "Did not build", "Could not launch"]


def categorize(status):
    for prefix, category in (("PASS", "Passed"), ("UPDATED", "Golden updated"),
                             ("DETERMINISTIC", "Deterministic"), ("FLAKY", "Flaky (passed on retry)")):
        if status.startswith(prefix):
            return category
    for text, category in (("not built", "Did not build"), ("timeout", "Timed out"),
                           ("cannot launch", "Could not launch"), ("no golden", "No golden"),
                           ("pixels differ", "Failed comparison"), ("image size differs", "Failed comparison"),
                           ("not deterministic", "Not deterministic"), ("no capture", "Capture failed"),
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
    gpus = sorted({r["gpu"] for r in results if r.get("gpu") and r["gpu"] != "unknown"})
    width = max([len(r["name"]) for r in rows] + [10])
    counts = {}
    for r in results:
        category = categorize(r["status"])
        counts[category] = counts.get(category, 0) + 1
    bad = [r for r in results if r["status"].startswith(("FAIL", "FLAKY"))]

    lines = ["OGLDEV regression run", "=====================", "",
             f"Date:        {started:%Y-%m-%d}",
             f"Time:        {started:%H:%M:%S}  (took {seconds // 60}m {seconds % 60:02d}s)",
             f"Git commit:  {commit}",
             f"Git branch:  {git('rev-parse', '--abbrev-ref', 'HEAD') or 'unknown'}"
             + (f"  (+ {len(modified)} uncommitted modified file(s))" if modified else ""),
             f"Machine:     {socket.gethostname()}",
             f"GPU:         {', '.join(gpus) or 'unknown'}",
             f"Mode:        {mode}",
             f"Build:       {build_line}",
             f"Command:     {' '.join(['regress.py'] + sys.argv[1:])}", ""]
    if missing:
        lines += manifest_warning(missing) + [""]

    lines += ["Summary", "-------", f"{'Projects run':36}{len(results):5}"]
    lines += [f"  {c:34}{counts[c]:5}" for c in CATEGORIES if c in counts]
    not_run = [r for r in rows if r not in selected]
    skipped = [r for r in not_run if r["status"] == "skip"]
    todo = [r for r in not_run if r["status"] == "todo"]
    lines += [f"{'Projects not run':36}{len(not_run):5}",
              f"  {'Excluded in the manifest':34}{len(skipped):5}",
              f"  {'Not enabled yet (todo)':34}{len(todo):5}",
              f"  {'Not selected for this run':34}{len(not_run) - len(skipped) - len(todo):5}",
              f"{'Projects missing from the manifest':36}{len(missing):5}", "",
              "Result:      " + ("FAILED" if any(r["status"].startswith("FAIL") for r in results) else "OK"), ""]

    if bad:
        lines += ["Problems", "--------"]
        lines += [f"{r['name']:{width}}  {r['status']}" for r in bad]
        lines += ["", f"Each project's own output is in {OUT.relative_to(REPO)}\\<project>\\log.txt", ""]

    lines += ["All projects that ran", "---------------------"]
    lines += [f"{r['name']:{width}}  {r['status']}" for r in results] or ["(none)"]
    lines += ["", "Excluded in the manifest", "------------------------"]
    lines += [f"{r['name']:{width}}  {r['note']}" for r in skipped] or ["(none)"]
    lines += ["", "Not enabled yet (todo)", "----------------------"]
    lines += [f"{r['name']:{width}}  {r['note']}" for r in todo] or ["(none)"]

    path = OUT / f"run_{started:%Y-%m-%d_%H-%M-%S}.log"
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return path


def set_status(rows, enable, disable):
    # --enable / --disable: switch manifest rows between 'todo' and 'ok', then exit
    by_name = {r["name"]: r for r in rows}
    unknown = [n for n in enable + disable if n not in by_name]
    if unknown:
        sys.exit(f"not in the manifest: {', '.join(unknown)}")
    both = set(enable) & set(disable)
    if both:
        sys.exit(f"both enabled and disabled: {', '.join(sorted(both))}")
    changed, refused = 0, 0
    for names, status in ((enable, "ok"), (disable, "todo")):
        for name in names:
            row = by_name[name]
            if row["status"] == "skip":
                refused += 1
                print(f"{name:32} not changed: excluded in the manifest ({row['note']})")
            elif row["status"] == status:
                print(f"{name:32} already '{status}'")
            else:
                print(f"{name:32} {row['status']} -> {status}")
                row["status"] = status
                changed += 1
            if status == "ok" and row["status"] == "ok" and not any(GOLD.glob(f"*/{name}/f*.png")):
                print(f"{'':32} note: it has no golden on this machine yet (--update-golden --only {name})")
    if changed:
        # Keep the line endings of the file so that only the edited rows show up in a diff
        newline = "\r\n" if b"\r\n" in MANIFEST.read_bytes() else "\n"
        with open(MANIFEST, "w", newline="") as f:
            w = csv.DictWriter(f, list(rows[0].keys()), lineterminator=newline)
            w.writeheader()
            w.writerows(rows)
    print(f"{changed} row(s) changed in {MANIFEST.relative_to(REPO)}")
    return 1 if refused else 0


def write_report(results):
    with open(OUT / "report.html", "w") as f:
        f.write("<meta charset=utf-8><title>Regression report</title>"
                "<table border=1 cellpadding=4><tr><th>frame<th>status<th>golden<th>actual<th>difference x8")
        for r in results:
            for name in r["frames"] or [""]:
                base = f"{r['name']}/{name}"
                f.write(f"<tr><td>{html.escape(base)}<td>{html.escape(r['status'])}")
                if not name:
                    f.write(f"<td colspan=3><a href='{html.escape(r['name'])}/log.txt'>log</a>")
                    continue
                f.write(f"<td><img src='../goldens/{r['gpu']}/{html.escape(base)}' width=260>"
                        f"<td><img src='{html.escape(base)}' width=260>"
                        f"<td><img src='{html.escape(base.replace('.png', '.diff.png'))}' width=260>")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--only", nargs="*", help="project names; also runs rows whose status is still 'todo'")
    ap.add_argument("--api")
    ap.add_argument("--tier")
    ap.add_argument("--update-golden", action="store_true")
    ap.add_argument("--retries", type=int, default=1)
    ap.add_argument("--strict", action="store_true")
    ap.add_argument("--gpu")
    ap.add_argument("--timeout", type=int, default=DEFAULT_TIMEOUT)
    ap.add_argument("--build", metavar="SLN", nargs="?", const=str(DEFAULT_SLN))
    ap.add_argument("--coverage", action="store_true")
    ap.add_argument("--audit", type=int, metavar="RUNS", help="run each project RUNS times and compare the captures byte for byte")
    ap.add_argument("--enable", nargs="+", default=[], metavar="NAME", help="set the manifest status of these projects to 'ok' and exit")
    ap.add_argument("--disable", nargs="+", default=[], metavar="NAME", help="set the manifest status of these projects back to 'todo' and exit")
    opt = ap.parse_args()

    with open(MANIFEST, newline="") as f:
        rows = list(csv.DictReader(f))
    if opt.enable or opt.disable:
        return set_status(rows, opt.enable, opt.disable)
    missing = missing_from_manifest(rows, opt.build or DEFAULT_SLN)
    for line in manifest_warning(missing):
        print(line, flush=True)
    if opt.coverage:
        for key in ("status", "tier", "api"):
            counts = {}
            for r in rows:
                counts[r[key] or "-"] = counts.get(r[key] or "-", 0) + 1
            print(key, counts)
        return 0
    if opt.only:
        unknown = set(opt.only) - {r["name"] for r in rows}
        if unknown:
            sys.exit(f"not in the manifest: {', '.join(sorted(unknown))}")
    started = datetime.datetime.now()
    OUT.mkdir(exist_ok=True)
    build_line, not_built = "not requested (existing executables were used)", set()
    if opt.build:
        build_line, not_built = build(opt.build, rows)
    selected = [r for r in rows
                if (r["name"] in opt.only and r["status"] != "skip" if opt.only else r["status"] == "ok")
                and (not opt.api or r["api"] == opt.api)
                and (not opt.tier or r["tier"] == opt.tier)]
    results, failed = [], 0
    if opt.audit:
        for r in selected:
            status = fail_status("not built") if r["name"] in not_built else \
                     audit(r, OUT / r["name"], opt.audit, opt.strict, opt.timeout)
            failed += status.startswith("FAIL")
            results.append({"name": r["name"], "status": status})
            print(f"{r['name']:32} {status}", flush=True)
        print(f"{len(selected)} audited, {failed} failed")
        log = write_run_log(started, opt, f"determinism audit, {opt.audit} runs per project", build_line, rows, selected, results, missing)
        print(f"log: {log.relative_to(REPO)}")
        return 1 if failed else 0
    for r in selected:
        outdir, attempt = OUT / r["name"], 0
        while True:
            attempt += 1
            code = "not built" if r["name"] in not_built else run_once(r, outdir, opt.strict, opt.timeout)
            if code != 0:
                status = fail_status(code)
                gpu, frames = opt.gpu or "unknown", []
            else:
                status, gpu, frames = check(r, outdir, opt.gpu, opt.update_golden)
            if not status.startswith("FAIL") or attempt > opt.retries or opt.update_golden or code == "not built":
                break
        if attempt > 1 and not status.startswith("FAIL"):
            status = "FLAKY (passed on retry)"
        failed += status.startswith("FAIL")
        results.append({"name": r["name"], "status": status, "gpu": gpu, "frames": frames})
        print(f"{r['name']:32} {status}", flush=True)
    write_report(results)
    (OUT / "results.json").write_text(json.dumps(results, indent=1))
    flaky = sum(r["status"].startswith("FLAKY") for r in results)
    print(f"{len(results)} run, {failed} failed, {flaky} flaky")
    mode = "update goldens" if opt.update_golden else "compare with goldens"
    log = write_run_log(started, opt, mode + (", strict" if opt.strict else ""), build_line, rows, selected, results, missing)
    print(f"log: {log.relative_to(REPO)}")
    return 1 if failed else 0


sys.exit(main())
