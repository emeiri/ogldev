# Generates Testing/manifest.csv from the application projects in a solution.
#
#   python Testing/make_manifest.py Windows\ogldev_vs_2026\ogldev_vs_2026.sln
#
# MSBuild evaluates the output path and the debugger settings of each project,
# so macros such as $(OutDir) are expanded for us. Rows that already exist in
# the manifest keep their hand-edited columns (api, frames, tol, ..., note).

import csv, json, os, re, subprocess, sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
SLN = Path(sys.argv[1]).resolve()
MANIFEST = REPO / "Testing" / "manifest.csv"
CONFIG = ["-p:Configuration=Debug", "-p:Platform=x64", f"-p:SolutionDir={SLN.parent}\\"]
PROPS = ["ConfigurationType", "TargetPath", "LocalDebuggerWorkingDirectory", "LocalDebuggerCommandArguments"]
COLUMNS = ["name", "api", "project", "exe", "cwd", "args", "frames", "tol", "max_bad", "mask", "tier", "status", "note"]


def find_msbuild():
    if os.environ.get("MSBUILD"):
        return os.environ["MSBUILD"]
    vswhere = Path(os.environ["ProgramFiles(x86)"]) / "Microsoft Visual Studio" / "Installer" / "vswhere.exe"
    r = subprocess.run([str(vswhere), "-latest", "-prerelease", "-requires", "Microsoft.Component.MSBuild",
                        "-find", r"MSBuild\**\Bin\MSBuild.exe"], capture_output=True, text=True)
    lines = r.stdout.split("\n")
    return lines[0].strip() if lines[0].strip() else "msbuild"


MSBUILD = find_msbuild()


def projects():
    text = SLN.read_text(encoding="utf-8-sig")
    pat = r'Project\("\{[^}]+\}"\) = "([^"]+)", "([^"]+\.vcxproj)"'
    for m in re.finditer(pat, text):
        yield m.group(1), (SLN.parent / m.group(2).replace("\\", "/")).resolve()


def evaluate(proj):
    cmd = [MSBUILD, str(proj), "-nologo", *CONFIG] + [f"-getProperty:{p}" for p in PROPS]
    r = subprocess.run(cmd, capture_output=True, text=True)
    try:
        return json.loads(r.stdout)["Properties"]
    except (ValueError, KeyError):
        print(f"warning: could not evaluate {proj}\n{r.stdout}{r.stderr}", file=sys.stderr)
        return None


def rel(path):
    # Repo-relative with forward slashes, so the manifest is the same on every machine.
    try:
        return Path(os.path.normpath(path)).relative_to(REPO).as_posix()
    except ValueError:
        return path


def main():
    projs = list(projects())
    with ThreadPoolExecutor(8) as pool:
        evaluated = list(pool.map(lambda p: evaluate(p[1]), projs))

    # Several projects share a name (there are three "Pong"s), so the manifest
    # name falls back to the project path relative to the solution.
    count = {}
    for name, _ in projs:
        count[name] = count.get(name, 0) + 1

    old = {}
    if MANIFEST.exists():
        with open(MANIFEST, newline="") as f:
            old = {r["name"]: r for r in csv.DictReader(f)}

    rows = []
    for (name, proj), props in zip(projs, evaluated):
        if props is None or props["ConfigurationType"] != "Application":
            continue
        if count[name] > 1:
            name = proj.parent.relative_to(SLN.parent).as_posix().replace("/", "_")
        row = dict.fromkeys(COLUMNS, "")
        row.update(frames="10", status="todo")
        row.update(old.get(name, {}))
        cwd = props["LocalDebuggerWorkingDirectory"] or str(proj.parent)
        if not os.path.isabs(cwd):
            cwd = str(proj.parent / cwd)
        row.update(name=name, project=rel(str(proj)), exe=rel(props["TargetPath"]), cwd=rel(cwd),
                   args=props["LocalDebuggerCommandArguments"])
        rows.append(row)

    MANIFEST.parent.mkdir(exist_ok=True)
    with open(MANIFEST, "w", newline="") as f:
        w = csv.DictWriter(f, COLUMNS)
        w.writeheader()
        w.writerows(rows)
    print(f"{len(rows)} application projects written to {rel(str(MANIFEST))}")


main()
