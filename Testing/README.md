# Golden-image regression suite

Builds nothing by itself: it launches the Debug x64 executables of the VS 2026 solution in a test mode, captures chosen frames from inside each application and compares them with locally stored golden images.

Everything that belongs to the suite is in this folder, except for a few headers and hooks listed under "What lives outside this folder".

## Usage

Run from the repo root. Needs `pip install pillow numpy`.

```powershell
# Create (or refresh) the goldens on this machine
python Testing\regress.py --update-golden

# Compare every enabled project with its golden
python Testing\regress.py

# A subset, by manifest name (this also runs rows that are still 'todo')
python Testing\regress.py --only Tutorial67_ProjectedTexture Tutorial17

# Build the solution first
python Testing\regress.py --build
```

Other options: `--api gl|vk`, `--tier N`, `--strict` (OpenGL/Vulkan errors fail the run), `--audit N` (run each project N times and require byte-identical captures), `--retries N`, `--timeout SECONDS`, `--gpu SLUG`, `--coverage`.

After a run open `Testing\out\report.html`: golden, actual and an amplified difference image side by side. `Testing\out\<project>\log.txt` has the application's output.

Every run also writes `Testing\out\run_<date>_<time>.log`, which is never overwritten. It records the date and time, the git commit and branch, the machine and GPU, the mode, the build result, a summary with the number of projects per outcome (passed, failed comparison, no golden, crashed, timed out, capture failed, did not build, ...) and the status of every project, including the ones the manifest excludes and why.

With `--build`, a failed solution build does not stop the run: the projects that failed to build are reported as "Did not build" and the rest are tested.

A project passes when it exits with code 0 and, for every captured frame, at most 0.1% of the pixels differ from the golden by more than 8/255 in any channel. `FLAKY` means it passed only on the retry.

## Files

| Path | Purpose |
| --- | --- |
| `regress.py` | The runner |
| `make_manifest.py` | Regenerates `manifest.csv` from the solution; hand-edited columns are kept |
| `manifest.csv` | One row per application project: its `.vcxproj`, how to launch it, frames to capture, tier, status |
| `shim/` | Wrapper headers for GLFW and freeglut (see below) |
| `goldens/<gpu>/<project>/fNNNN.png` | Golden images. Local to a machine, not committed |
| `out/` | Captures, logs and reports of the last run. Not committed |

Manifest `status` is `ok` (runs by default), `todo` (runs only with `--only`) or `skip` (never runs; the reason is in `note`).

`--enable NAME...` and `--disable NAME...` switch rows between `todo` and `ok` in the manifest and exit without running anything. `skip` rows are left alone.

```powershell
python Testing\regress.py --enable Tutorial27 Tutorial29
python Testing\regress.py --disable Tutorial27
```

## Adding a project to the solution

The test mode reaches a new project by itself, but the runner only tests what is in the manifest. At the start of every run it compares the solution with the manifest and prints a warning, also written to the run log, that lists the application projects that have no row. To add them:

```powershell
python Testing\make_manifest.py Windows\ogldev_vs_2026\ogldev_vs_2026.sln   # adds the new rows as 'todo'
python Testing\regress.py --audit 3 --only MyNewProject                      # is it repeatable?
python Testing\regress.py --update-golden --only MyNewProject                # create its golden, then look at it
```

Then enable it with `python Testing\regress.py --enable MyNewProject`.

## How the test mode gets into the applications

The tutorials contain no testing code. `Windows\ogldev_vs_2026\Directory.Build.targets` is imported by MSBuild into every project of the solution and puts `Testing\shim` in front of the include path. `<GLFW/glfw3.h>` and `<GL/freeglut.h>` therefore resolve to the wrappers in `shim/`, which include the real header and then redirect a few calls:

- `glfwSwapBuffers` / `glutSwapBuffers`: end of the frame. Captures the back buffer on the requested frames and closes the application after the last one.
- `glfwGetTime` / `glutGet(GLUT_ELAPSED_TIME)`: a fixed clock that advances 1/60 s per frame.
- input callbacks and full screen: disabled.

Unless the application is started with `--test-frames`, every wrapper just calls the real function.

Options understood by an application (the runner passes them):

- `--test-frames 10,60,200`: capture these frames and exit with code 0 after the last one
- `--screenshot-dir DIR`: where `fNNNN.png` and `info.json` go
- `--test-seed S`: seed for `rand()`
- `--test-strict`: exit with code 3 on an OpenGL debug error or a Vulkan validation error

## What lives outside this folder

- `Include/regress_hook.h`, `Include/regress_capture_gl.h`: the test mode itself and the OpenGL capture.
- `Windows/ogldev_vs_2026/Directory.Build.targets`: adds the shim to the include path.
- `Common/ogldev_util.cpp`: `GetCurrentTimeMillis` returns the test clock; `glDebugOutput` reports errors for `--test-strict`.
- `Common/ogldev_base_app.cpp`, `ogldev_base_app2.cpp`, `DemoLITION/Framework/Source/core_rendering_system.cpp`: 800x600 window in test mode (these demos ask for windows larger than most screens).
- `Common/3rdparty/ImGui/GLFW/imgui_impl_opengl3.cpp`: no GUI in test mode, because it shows frame rates.
- `Vulkan/VulkanCore`: swapchain capture in `VulkanCore::CaptureSwapChainImage`, called from `VulkanQueue::Present`.

The Linux build does not use the shim, so the test mode is Windows only.
