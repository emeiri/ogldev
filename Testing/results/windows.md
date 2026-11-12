# Regression results: windows

Testing executed on Windows PC (`windows`), commit `1b4901be` Compare against the three platforms (+3 uncommitted).

**Result: PASS**: 132 run, 132 passed, 0 failed.

Other machines (larger tolerance, not part of the result):
- `linux-t420`, Lenovo ThinkPad T420 (4 cores): 90 match, 3 differ, 13 no golden, 26 not compared
- `linux-ideapad`, Lenovo IdeaPad (8 cores): 91 match, 1 differ, 14 no golden, 26 not compared

## Differences from the other machines

| Project                  | Golden set    | Frame | Pixels differ | Note                                                   |
|--------------------------|---------------|------:|--------------:|--------------------------------------------------------|
| Tutorial18_LoadingModels | linux-t420    |    10 |         2.06% |                                                        |
| Tutorial31_3DPicking     | linux-t420    |    10 |         2.11% |                                                        |
| PongVK                   | linux-t420    |    10 |        19.19% | cropped: 2560x1415 and 2560x1440 compared at 2516x1415 |
| PongVK                   | linux-ideapad |    10 |        19.19% | cropped: 2560x1415 and 2560x1440 compared at 2516x1415 |

