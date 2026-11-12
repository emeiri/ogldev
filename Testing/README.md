# Golden-image regression suite

Builds nothing by itself (unless asked to): it launches the executables of the VS 2026 solution on Windows, or of the Makefiles on Linux, in a test mode, captures chosen frames from inside each application and compares them with golden images.

Each machine has its own goldens (`goldens/<machine>/`). The runner detects the machine it runs on (see "Machines"), and every run compares with all the golden sets: strictly with the goldens of this machine and with a larger tolerance with the goldens of every other machine.

Everything that belongs to the suite is in this folder, except for a few headers and hooks listed under "What lives outside this folder".

## Usage

Run from the repo root.

- Windows: needs `pip install pillow numpy`.
- Linux: needs `sudo apt install python3-pil python3-numpy xvfb mesa-utils`, and the projects built with `make`.

```
# Create (or refresh) the goldens of this machine
python Testing/regress.py --update-golden

# Compare every enabled project with the goldens
python Testing/regress.py

# A subset, by manifest name (this also runs rows that are still 'todo')
python Testing/regress.py --only Tutorial67_ProjectedTexture Tutorial17

# Build first (the solution on Windows, 'make' on Linux)
python Testing/regress.py --build
```

On Linux use `python3`.

Other options: `--window WxH|native` (see below), `--api gl|vk`, `--tier N`, `--strict` (OpenGL/Vulkan errors fail the run), `--audit N` (run each project N times and require byte-identical captures), `--retries N`, `--timeout SECONDS`, `--machine NAME` (don't detect the machine, see "Machines"), `--capture hw|sw|sw-gl46` (Linux, see below), `--coverage`.

After a run open `Testing/out/report.html`: this machine's golden, actual and an amplified difference image side by side, then each other machine's golden and its difference image. `Testing/out/<project>/log.txt` has the application's output.

Every run also writes `Testing/out/run_<date>_<time>.log`, which is never overwritten. It records the date and time, the git commit and branch, the machine, platform and GPU, the capture methods, the mode, the build result, a summary with the number of projects per outcome (passed, failed comparison, failed comparison with another machine, no golden, crashed, timed out, capture failed, did not build, ...) and the status of every project, including the ones the manifest excludes and why.

At the end of a run the runner prints a summary and writes it to `Testing/results/<machine>.md`, meant to be committed. It says which machine the run was on, the commit and the GPU, and has one row per golden set, this machine's first: how many projects were run, passed, failed, were flaky, had no golden, were not compared or were skipped, followed by the problems with each golden set. This machine's row counts the outcome of each project; the other rows count how its captures compare with that machine's goldens. The summary is also appended to the run log.

## Machines

`machines.csv` lists the machines that have goldens. `name` is the folder under `goldens/`, `platform` is `windows` or `linux`, and `description` appears in the summary. The runner picks the row of its platform whose `product` is part of the computer's vendor and model as the firmware reports them (`/sys/class/dmi/id` on Linux, the BIOS registry key on Windows; case is ignored), or else the row whose `cpus` is the number of logical CPUs (`nproc`). A platform with a single row and neither column needs no detection. `--machine NAME`, or the environment variable `OGLDEV_MACHINE`, chooses the row instead.

| Machine | Goldens |
| --- | --- |
| Windows PC | `goldens/windows` |
| Lenovo ThinkPad T420 (4 cores) | `goldens/linux-t420` |
| Lenovo IdeaPad (8 cores) | `goldens/linux-ideapad` |

All machines of a platform use that platform's columns of the manifest. To add a machine, add its row and create its goldens with `--update-golden`.

With `--build`, a failed build does not stop the run: the projects that failed to build are reported as "Did not build" and the rest are tested.

## Passing

A project passes when it exits with code 0 and, for every captured frame:

- at most 0.1% of the pixels differ from the golden of this machine by more than 8/255 in any channel (columns `tol` and `max_bad` override this), and
- at most 2% of the pixels differ from the golden of each other machine by more than 48/255 (columns `cross_tol` and `cross_max_bad`). A missing golden of another machine is reported (`no linux-t420 golden`) but is not a failure. A project that exists on only one platform is reported as `linux only` or `windows only`.

The goldens of two machines may have different sizes, because each machine captures every window at the size the project asks for and a window manager may shrink a window that doesn't fit the screen. The comparison with another machine then scales the larger image down to the size of the smaller one. When the aspect ratios differ, it tries both stretching the image and cropping its center to the same aspect ratio, which matches a projection that keeps its vertical or horizontal field of view, and keeps whichever is closer. The status says which one was used, e.g. `(windows golden: 0.13% differ, scaled: 1600x843 and 1920x1080 compared at 1600x843)`.

`cross_max_bad` = `off` turns the comparison with the other machines off for a row whose image cannot match. The note gives the reason. Random numbers are not such a reason: in test mode `RANDOM`, `SRANDOM` and `RandomFloat` (Include/ogldev_util.h) use `Regress::Rand()`, which gives the same sequence on every platform. A program that calls `rand()` directly still gets the sequence of its C runtime.

`FLAKY` means it passed only on the retry.

## Capture on Linux

A project is captured in one of these ways:

| Method | How |
| --- | --- |
| `hw` | The GPU, on the X display of the desktop session (`$DISPLAY`, else `:0`; `OGLDEV_HW_DISPLAY` overrides it) |
| `sw` | Mesa's software renderers, llvmpipe for OpenGL and lavapipe for Vulkan, on a private Xvfb |
| `sw-gl46` | The same, with llvmpipe reporting OpenGL 4.6 (it implements 4.5, the shaders of some projects ask for 4.60) |

The method is chosen when a project's golden is first created on a Linux machine and is recorded in that golden's `golden.json`, so each machine has its own. Later runs on that machine always use the recorded method. To choose it, the runner tries `hw` in strict mode and accepts it when the project runs on a hardware renderer with no OpenGL errors and its window kept its size (a window larger than the screen is shrunk by the window manager, so such a project is captured with `sw`, on a 2560x1600 Xvfb). Otherwise it tries `sw`, then `sw-gl46`. `Testing/out/<project>/methods.txt` says why a method was rejected. `--capture METHOD` with `--update-golden` sets the method, e.g. `--capture sw` to create goldens while the desktop is locked. To choose again, run `--update-golden --capture METHOD --only NAME`. The manifest column `linux_capture` is no longer used.

`hw` needs the desktop session to be unlocked and on screen. A locked session (light-locker switches to the greeter) stops the GPU rendering for it. During a run the screen saver and DPMS are switched off and restored at the end. If the display is not available, `hw` projects are reported as skipped, and new projects are not given a method rather than falling back to software.

`golden.json` next to each golden records the platform, machine, method, renderer, OpenGL/Vulkan version, date and commit it was made with.

## Files

| Path | Purpose |
| --- | --- |
| `regress.py` | The runner |
| `make_manifest.py` | Regenerates `manifest.csv` from the solution (Windows) or adds the Linux projects (`--linux`); hand-edited columns are kept |
| `manifest.csv` | One row per application: how to launch it on each platform, frames to capture, tolerances, tier, status |
| `machines.csv` | The machines that have goldens and how to recognize them (see "Machines") |
| `shim/` | Wrapper headers for GLFW and freeglut (see below) |
| `goldens/windows/<project>/fNNNN.png` | Golden images made on the Windows PC (NVIDIA GeForce GTX 1650) |
| `goldens/linux-t420/<project>/fNNNN.png` | Golden images made on the ThinkPad T420 (Intel HD Graphics 3000, or llvmpipe/lavapipe) |
| `goldens/linux-ideapad/<project>/fNNNN.png` | Golden images made on the IdeaPad |
| `results/<machine>.md` | Summary of the last run on each machine, to commit |
| `out/` | Captures, logs and reports of the last run. Not committed |

The manifest columns `project`, `exe`, `cwd`, `status` and `note` describe the Windows project. `linux_dir` (the directory of its Makefile), `linux_cwd`, `linux_status` and `linux_note` describe the Linux one (`linux_capture` is no longer used). A row may have only one of them: some rows are Windows only, and the rows named `linux_*` are Linux only.

Status (`status` on Windows, `linux_status` on Linux) is `ok` (runs by default), `todo` (runs only with `--only`) or `skip` (never runs; the reason is in the note).

`--enable NAME...` and `--disable NAME...` switch rows of the current platform between `todo` and `ok` in the manifest and exit without running anything. `skip` rows are left alone.

```
python Testing/regress.py --enable Tutorial27 Tutorial29
python Testing/regress.py --disable Tutorial27
```

## Adding a project

The test mode reaches a new project by itself, but the runner only tests what is in the manifest. At the start of every run it compares the projects with the manifest and prints a warning, also written to the run log, that lists the projects that have no row. To add them:

```
python Testing\make_manifest.py Windows\ogldev_vs_2026\ogldev_vs_2026.sln   # Windows: adds the new rows as 'todo'
python3 Testing/make_manifest.py --linux                                    # Linux: the same for the Makefiles
python Testing/regress.py --audit 3 --only MyNewProject                     # is it repeatable?
python Testing/regress.py --update-golden --only MyNewProject               # create its golden, then look at it
```

Then enable it with `python Testing/regress.py --enable MyNewProject`. A project that exists on both platforms should have one row with both sets of columns, so that its captures are compared with the goldens of every machine.

## How the test mode gets into the applications

The tutorials contain no testing code. On Windows `Windows\ogldev_vs_2026\Directory.Build.targets` is imported by MSBuild into every project of the solution and puts `Testing\shim` in front of the include path. On Linux `Make/config.mk` does the same. `<GLFW/glfw3.h>` and `<GL/freeglut.h>` therefore resolve to the wrappers in `shim/`, which include the real header and then redirect a few calls:

- `glfwSwapBuffers` / `glutSwapBuffers`: end of the frame. Captures the back buffer on the requested frames and closes the application after the last one.
- `glfwGetTime` / `glutGet(GLUT_ELAPSED_TIME)`: a fixed clock that advances 1/60 s per frame.
- `glfwCreateWindow` / `glutInitWindowSize` / `glutEnterGameMode`: never full screen. An OpenGL window keeps the size the project asks for, unless the runner passes `--window WxH` (then `--test-window WxH`). Many tutorials size their viewport and projection from their own constants, so in a smaller window they draw only part of the image. Vulkan windows always keep their size for the same reason. The frameworks of `ogldev_base_app` and DemoLITION ask for windows larger than most screens and use 800x600 in test mode (or the `--test-window` size).
- input callbacks: disabled.

Unless the application is started with `--test-frames`, every wrapper just calls the real function.

Options understood by an application (the runner passes them):

- `--test-frames 10,60,200`: capture these frames and exit with code 0 after the last one
- `--screenshot-dir DIR`: where `fNNNN.png` and `info.json` go
- `--test-seed S`: seed for `rand()` and `Regress::Rand()`
- `--test-strict`: exit with code 3 on an OpenGL debug error or a Vulkan validation error
- `--test-window 800x600`: the size of the OpenGL windows

## What lives outside this folder

- `Include/regress_hook.h`, `Include/regress_capture_gl.h`: the test mode itself and the OpenGL capture.
- `Windows/ogldev_vs_2026/Directory.Build.targets` (Windows) and `Make/config.mk` (Linux): add the shim to the include path.
- `Common/ogldev_util.cpp`: `GetCurrentTimeMillis` returns the test clock; `glDebugOutput` reports errors for `--test-strict`.
- `Common/ogldev_base_app.cpp`, `ogldev_base_app2.cpp`, `DemoLITION/Framework/Source/core_rendering_system.cpp`: 800x600 window in test mode, or the `--test-window` size (these demos ask for windows larger than most screens).
- `Common/3rdparty/ImGui/GLFW/imgui_impl_opengl3.cpp`: no GUI in test mode, because it shows frame rates.
- `Vulkan/VulkanCore`: swapchain capture in `VulkanCore::CaptureSwapChainImage`, called from `VulkanQueue::Present`.
