<callout icon="✅" color="green_bg">
	**2026-10-18 · DX11 13A 현재 Notion에 저장된 4개 C++ 코드 블록을 직접 복원해 Linux CPU 재검증 완료.** 이전의 원본 SHA 불일치는 여전히 역사적 불일치로 보존하며 이번 검증과 구분합니다.
</callout>
## 원본 코드 복원·검증 기준
- 상위 13A <mention-page url="https://app.notion.com/p/3fba99703f40810b826cc7eec5b1f039">13A · Instancing CPU 구현·검증 색인과 전체 소스 보관</mention-page>의 `main.cpp` 4개 코드 블록(전문 1/3, 2/3, 3a/3c, 3b/3c)을 표시 순서대로 연결하되 **각 블록 뒤에 정확히 LF(n) 1개**를 덧붙입니다. UTF-8/ASCII 6,043바이트.
- 이 규칙으로 재조회된 실제 코드의 **SHA-256 ****`ef9b0cfe2e764adc896a45451f5607fd751ab0c8b523e6477339994ba758a4b8`****, FNV64 ****`4c27d09435055a79`**. 별도 컨테이너에 같은 바이트를 재구성한 뒤 SHA-256과 FNV64 둘 다 일치함을 확인했습니다.
- 13A의 CMake 513바이트 SHA-256 `1ba7395b7b9201cf982fa1059a41a70edef99fbd4aa93743088b4dc43a3c3ed2`도 실제 재구성 파일과 기록값이 일치했습니다.
- 과거 기록된 `main.cpp` SHA-256 `fd3839f7c350d1e573cab36a8c3f942f55a8e6c9d4e94b45033e7d3b3b4472ae`와 **현재 Notion 버전은 서로 다릅니다.** 역사적 원본 동일성을 주장하지 않습니다. 이번 실행이 검증한 것은 **현재 보관된 6,043바이트 소스의 재현 가능한 컴파일·CPU 실행**입니다.
## 재현 빌드 명령
```bash
# 13A에 저장된 main.cpp와 CMakeLists.txt를 위 규칙으로 복원한 디렉터리에서 실행
CXX=g++ cmake -S . -B build-gcc20 -DLAB_CXX_STANDARD=20
cmake --build build-gcc20
ctest --test-dir build-gcc20 -V
CXX=g++ cmake -S . -B build-gcc11 -DLAB_CXX_STANDARD=11
cmake --build build-gcc11
ctest --test-dir build-gcc11 -V
# 기존 캐시를 재사용하지 않고 첫 Configure부터 sanitizer 플래그 설정
CXX=clang++ CXXFLAGS='-fsanitize=address,undefined -fno-omit-frame-pointer' LDFLAGS='-fsanitize=address,undefined' cmake -S . -B build-clangsan -DLAB_CXX_STANDARD=20
cmake --build build-clangsan
ctest --test-dir build-clangsan -V
```
## 실제 검증 결과
<table header-row="true">
<tr>
<td>환경</td>
<td>CPU 검사</td>
<td>CTest</td>
<td>검증 특기</td>
</tr>
<tr>
<td>GCC 14.2 C++20</td>
<td>37조건</td>
<td>1/1 PASS</td>
<td>-Wall -Wextra -Wpedantic -Werror</td>
</tr>
<tr>
<td>GCC 14.2 C++11</td>
<td>동일 37조건</td>
<td>1/1 PASS</td>
<td>호환 컴파일</td>
</tr>
<tr>
<td>Clang 17 C++20 ASan/UBSan</td>
<td>동일 37조건</td>
<td>1/1 PASS</td>
<td>CMAKE_CXX_FLAGS 및 실제 컴파일 flags.make의 -fsanitize 확인</td>
</tr>
</table>
3환경 반복을 독립적인 **111개** 테스트로 집계하지 않습니다. Windows/MSVC, DirectX11 GPU, HLSL FXC/DXC, 드로우 호출 및 성능은 **미검증**입니다.
## 원시 로그 (Configure·Build·CTest -V, 합계 9개)
### gcc20-configure.log · 401바이트 · FNV64 110f3931d17d0ccf
```plain text
-- The CXX compiler identification is GNU 14.2.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/g++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done (0.2s)
-- Generating done (0.0s)
-- Build files have been written to: /mnt/data/dx11_13a_current_notionsource/build-gcc20
```
### gcc20-build.log · 139바이트 · FNV64 65ed3512460ddc55
```plain text
[ 50%] Building CXX object CMakeFiles/dx11_13_cpu.dir/main.cpp.o
[100%] Linking CXX executable dx11_13_cpu
[100%] Built target dx11_13_cpu
```
### gcc20-ctest.log · 1002바이트 · FNV64 8b11cbc773c42a28
```plain text
Internal ctest changing into directory: /mnt/data/dx11_13a_current_notionsource/build-gcc20
UpdateCTestConfiguration  from :/mnt/data/dx11_13a_current_notionsource/build-gcc20/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/dx11_13a_current_notionsource/build-gcc20/DartConfiguration.tcl
Test project /mnt/data/dx11_13a_current_notionsource/build-gcc20
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: dx11_13_cpu

1: Test command: /mnt/data/dx11_13a_current_notionsource/build-gcc20/dx11_13_cpu
1: Working Directory: /mnt/data/dx11_13a_current_notionsource/build-gcc20
1: Test timeout computed to be: 10000000
1: PASS 37 DX11 instancing CPU contract checks
1/1 Test #1: dx11_13_cpu ......................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.00 sec
```
### gcc11-configure.log · 401바이트 · FNV64 f7efbe31c380d967
```plain text
-- The CXX compiler identification is GNU 14.2.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/g++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done (0.2s)
-- Generating done (0.0s)
-- Build files have been written to: /mnt/data/dx11_13a_current_notionsource/build-gcc11
```
### gcc11-build.log · 139바이트 · FNV64 65ed3512460ddc55
```plain text
[ 50%] Building CXX object CMakeFiles/dx11_13_cpu.dir/main.cpp.o
[100%] Linking CXX executable dx11_13_cpu
[100%] Built target dx11_13_cpu
```
### gcc11-ctest.log · 1002바이트 · FNV64 d8b509b05b5a6948
```plain text
Internal ctest changing into directory: /mnt/data/dx11_13a_current_notionsource/build-gcc11
UpdateCTestConfiguration  from :/mnt/data/dx11_13a_current_notionsource/build-gcc11/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/dx11_13a_current_notionsource/build-gcc11/DartConfiguration.tcl
Test project /mnt/data/dx11_13a_current_notionsource/build-gcc11
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: dx11_13_cpu

1: Test command: /mnt/data/dx11_13a_current_notionsource/build-gcc11/dx11_13_cpu
1: Working Directory: /mnt/data/dx11_13a_current_notionsource/build-gcc11
1: Test timeout computed to be: 10000000
1: PASS 37 DX11 instancing CPU contract checks
1/1 Test #1: dx11_13_cpu ......................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.00 sec
```
### clangsan-configure.log · 426바이트 · FNV64 5e410513310d952b
```plain text
-- The CXX compiler identification is Clang 17.0.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/local/swift/usr/bin/clang++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done (0.5s)
-- Generating done (0.0s)
-- Build files have been written to: /mnt/data/dx11_13a_current_notionsource/build-clangsan
```
### clangsan-build.log · 139바이트 · FNV64 65ed3512460ddc55
```plain text
[ 50%] Building CXX object CMakeFiles/dx11_13_cpu.dir/main.cpp.o
[100%] Linking CXX executable dx11_13_cpu
[100%] Built target dx11_13_cpu
```
### clangsan-ctest.log · 1020바이트 · FNV64 b9f926f696d8394e
```plain text
Internal ctest changing into directory: /mnt/data/dx11_13a_current_notionsource/build-clangsan
UpdateCTestConfiguration  from :/mnt/data/dx11_13a_current_notionsource/build-clangsan/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/dx11_13a_current_notionsource/build-clangsan/DartConfiguration.tcl
Test project /mnt/data/dx11_13a_current_notionsource/build-clangsan
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: dx11_13_cpu

1: Test command: /mnt/data/dx11_13a_current_notionsource/build-clangsan/dx11_13_cpu
1: Working Directory: /mnt/data/dx11_13a_current_notionsource/build-clangsan
1: Test timeout computed to be: 10000000
1: PASS 37 DX11 instancing CPU contract checks
1/1 Test #1: dx11_13_cpu ......................   Passed    0.01 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.01 sec
```
## 변경 이력 및 미완료
- 본 문서는 기존 13A의 코드를 수정하거나 과거 해시를 덮어쓰지 않고 **재검증 로그와 추적 가능한 기준값**을 하위에 영구 저장합니다.
- 기존 소스의 Layout Element 오버랩 계산이 아직 검증되지 않은 후속 Element의 offset+bytes를 먼저 더하는 구조는 임의 큰 unsigned 입력에 견고하지 않을 수 있습니다. 실제 GPU InputLayout 구현과 혼동하지 말고, 후속 개선 시 전 Element bounds 검증 다음 overlap을 검사하세요.
- 2026-10-19 문서 상태 재점검: 원시 Configure·Build·CTest 로그 아홉 블록이 이 페이지 본문에 존재함을 확인했습니다. 이전의 ‘로그 페이지 생성 후 검증 필요’ 안내는 현재 문서 상태를 반영하지 못합니다. 이번 점검에서는 각 로그를 로컬 원본과 재해시하지 않았으므로 바이트 무결성의 신규 검증으로 집계하지 않습니다. 과거 main.cpp SHA 불일치 및 Windows/D3D11 GPU 미검증 상태는 유지합니다.
