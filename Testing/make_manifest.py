# Generates Testing/manifest.csv from the application projects in a solution.
#
#   python Testing/make_manifest.py Windows\ogldev_vs_2026\ogldev_vs_2026.sln
#   python3 Testing/make_manifest.py --linux
#
# MSBuild evaluates the output path and the debugger settings of each project,
# so macros such as $(OutDir) are expanded for us. Rows that already exist in
# the manifest keep their hand-edited columns (api, frames, tol, ..., note) and
# their Linux columns. Rows that exist only on Linux are kept as they are.
#
# --linux adds a 'todo' row for every Makefile project (a directory whose
# Makefile includes Make/project.mk) that no row refers to yet. If the project
# also exists on Windows, move its linux_* columns to the Windows row by hand
# so that the two platforms are compared.

import csv, json, os, re, subprocess, sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
MANIFEST = REPO / "Testing" / "manifest.csv"
PROPS = ["ConfigurationType", "TargetPath", "LocalDebuggerWorkingDirectory", "LocalDebuggerCommandArguments"]
COLUMNS = ["name", "api", "project", "exe", "cwd", "args", "frames", "tol", "max_bad", "mask", "tier", "status", "note",
           "linux_dir", "linux_cwd", "linux_status", "linux_capture", "linux_note", "cross_tol", "cross_max_bad"]


def find_msbuild():
    if os.environ.get("MSBUILD"):
        return os.environ["MSBUILD"]
    vswhere = Path(os.environ["ProgramFiles(x86)"]) / "Microsoft Visual Studio" / "Installer" / "vswhere.exe"
    r = subprocess.run([str(vswhere), "-latest", "-prerelease", "-requires", "Microsoft.Component.MSBuild",
                        "-find", r"MSBuild\**\Bin\MSBuild.exe"], capture_output=True, text=True)
    lines = r.stdout.split("\n")
    return lines[0].strip() if lines[0].strip() else "msbuild"


def projects(sln):
    text = sln.read_text(encoding="utf-8-sig")
    pat = r'Project\("\{[^}]+\}"\) = "([^"]+)", "([^"]+\.vcxproj)"'
    for m in re.finditer(pat, text):
        yield m.group(1), (sln.parent / m.group(2).replace("\\", "/")).resolve()


def evaluate(msbuild, sln, proj):
    config = ["-p:Configuration=Debug", "-p:Platform=x64", f"-p:SolutionDir={sln.parent}\\"]
    cmd = [msbuild, str(proj), "-nologo", *config] + [f"-getProperty:{p}" for p in PROPS]
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


def read_manifest():
    if not MANIFEST.exists():
        return []
    with open(MANIFEST, newline="") as f:
        return list(csv.DictReader(f))


def write_manifest(rows):
    MANIFEST.parent.mkdir(exist_ok=True)
    with open(MANIFEST, "w", newline="") as f:
        w = csv.DictWriter(f, COLUMNS, extrasaction="ignore", lineterminator="\n")
        w.writeheader()
        w.writerows(rows)


def linux_main():
    rows = read_manifest()
    known = {r.get("linux_dir", "") for r in rows}
    names = {r["name"].lower() for r in rows}
    added = 0
    for d, dirs, files in sorted(os.walk(REPO)):
        dirs[:] = [x for x in dirs if x not in (".git", "build")]
        if "Makefile" not in files or Path(d) == REPO:
            continue
        text = (Path(d) / "Makefile").read_text(errors="replace")
        rel_dir = Path(d).relative_to(REPO).as_posix()
        if "Make/project.mk" not in text or rel_dir in known:
            continue
        # Manifest names must be unique ignoring case: the goldens are folders
        name = "linux_" + rel_dir.replace("/", "_")
        if name.lower() in names:
            continue
        row = dict.fromkeys(COLUMNS, "")
        row.update(name=name, api="vk" if re.search(r"^API\s*:=\s*vulkan", text, re.M) else "gl", frames="10",
                   linux_dir=rel_dir, linux_cwd=rel_dir, linux_status="todo")
        rows.append(row)
        names.add(name.lower())
        added += 1
    write_manifest(rows)
    print(f"{added} Linux project(s) added to {rel(str(MANIFEST))} as 'todo'")


def main():
    if len(sys.argv) > 1 and sys.argv[1] == "--linux":
        return linux_main()
    sln = Path(sys.argv[1]).resolve()
    msbuild = find_msbuild()
    projs = list(projects(sln))
    with ThreadPoolExecutor(8) as pool:
        evaluated = list(pool.map(lambda p: evaluate(msbuild, sln, p[1]), projs))

    # Several projects share a name (there are three "Pong"s), so the manifest
    # name falls back to the project path relative to the solution.
    count = {}
    for name, _ in projs:
        count[name] = count.get(name, 0) + 1

    old_rows = read_manifest()
    old = {r["name"]: r for r in old_rows}

    rows = []
    for (name, proj), props in zip(projs, evaluated):
        if props is None or props["ConfigurationType"] != "Application":
            continue
        if count[name] > 1:
            name = proj.parent.relative_to(sln.parent).as_posix().replace("/", "_")
        row = dict.fromkeys(COLUMNS, "")
        row.update(frames="10", status="todo")
        row.update(old.get(name, {}))
        cwd = props["LocalDebuggerWorkingDirectory"] or str(proj.parent)
        if not os.path.isabs(cwd):
            cwd = str(proj.parent / cwd)
        row.update(name=name, project=rel(str(proj)), exe=rel(props["TargetPath"]), cwd=rel(cwd),
                   args=props["LocalDebuggerCommandArguments"])
        rows.append(row)

    # The rows that exist only on Linux
    linux_only = [r for r in old_rows if not r.get("project") and r.get("linux_dir")]
    write_manifest(rows + linux_only)
    print(f"{len(rows)} application projects written to {rel(str(MANIFEST))}, {len(linux_only)} Linux only rows kept")


main()
