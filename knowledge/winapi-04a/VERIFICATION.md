# WinAPI04A · migration-specific CPU verification (2026-10-19)

Source input: GitHub `knowledge/winapi-04a/main.cpp` and `CMakeLists.txt`, byte-matched to the original Notion native blocks before commit (see `SOURCE_VALIDATION.md`).

New run: [GitHub Actions #39207867802](https://github.com/jiwhan112/um.github.io/actions/runs/39207867802). The three complete decoded GitHub Actions job logs are preserved alongside this note as **unedited raw logs**. Each log includes original configure, build and CTest console output. These are **new GitHub CI executions**, not copies of Notion's historical test outputs.

| Configuration | Job | CTest | Unique logical checks |
|---|---|---|---|
| GCC C++20 | [117914533627](https://github.com/jiwhan112/um.github.io/actions/runs/39207867802/job/117914533627) | 1/1 passed | 23 |
| GCC C++11 | [117914533536](https://github.com/jiwhan112/um.github.io/actions/runs/39207867802/job/117914533536) | 1/1 passed | same 23 repeated |
| Clang C++20 ASan/UBSan | [117914533354](https://github.com/jiwhan112/um.github.io/actions/runs/39207867802/job/117914533354) | 1/1 passed | same 23 repeated |

Each compiler ran the **same 23 logical conditions**. Reporting 69 independent checks would be double-counting. The sanitizer run displayed no sanitizer diagnostic in its passed CTest. GitHub CI Linux only; Windows GUI, Win32 message pump, MFC/D3D9/GPU, and real frame timings were not executed here.

Commands are specified in `.github/workflows/knowledge-cpu-check.yml`; build artifacts are not runtime dependencies. Reported stdout results are backed by the linked job logs and these permanently committed text copies.
