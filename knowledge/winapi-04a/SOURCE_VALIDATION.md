# WinAPI04A source integrity

- Notion source: https://app.notion.com/p/3fda99703f4081b18c16d94ceb844473
- `main.cpp` comprises original 2450+2251 byte halves, joined without extra bytes, exactly 4701 bytes; FNV64 `b51d90ade18fcc1e`, original Notion SHA256 `5e711d0e1b1685cca876e94c3416dde0e4271c3bb4e9e959fc292f2d7dde15d2` (SHA needs independent verification).
- `CMakeLists.txt`: exactly 543 bytes; FNV64 `7bf11d85adb99365`.
- Both FNV64 values checked against actual Notion native code blocks before this GitHub commit.
- C++11 and C++20 intended. Windows Win32/GDI/Direct3D tests not run as part of the extraction.
- Independent GitHub CI tests and raw logs are reported separately, never passed off as Notion historical tests.
