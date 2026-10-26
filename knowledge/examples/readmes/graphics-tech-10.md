# 그래픽스 기술 10 · 가시성·LOD·히스테리시스·성능 검증

## 무엇을 실행하나

평면과 bounding sphere 관계로 보수적 가시성을 판정하고, 화면 크기 기준 LOD의 상태 전환을 만든다. 후보 수 감소·CPU 연산 수·실제 프레임 시간을 다른 지표로 보고한다.

평면은 dot(n,p)+d≥0이 안쪽이다. 법선이 단위 벡터가 아니어도 반지름에 |n|을 곱한다. 예제의 6평면은 투영 행렬에서 추출한 frustum이 아니라 [-1,1] 상자다. 화면 반지름은 정면의 작은 원판 근사이며 sphere 실루엣 정확식이 아니다. occlusion query와 GPU 실행은 없다.

이 프로젝트는 C++17 CPU 수치 실습이다. 실제 DirectX 장치·GPU 셰이더·렌더링 창을 만들지 않는다. 프로그램은 표준입력, 외부 파일, 인터넷을 사용하지 않으며 실행 파일은 ZIP에 포함하지 않는다.

## 준비와 실행

C++17 컴파일러와 CMake 3.16 이상이 필요하다. ZIP을 빈 폴더에 풀고 README 위치에서 실행한다.

```sh
cmake -S project -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
./build/graphics_lab
```

Windows의 multi-config generator에서는 `cmake --build build --config Debug`와 `ctest --test-dir build -C Debug --output-on-failure`를 사용하고 `build/Debug/graphics_lab.exe`를 실행한다. 이것은 실행 안내이며 이번 Windows/MSVC 검증 결과는 아니다.

기대 출력(부동소수점 마지막 자리는 구현에 따라 조금 달라질 수 있다):

```text
candidates=5 of 21; CPU plane tests=54; no GPU timing measured
PASS: 11 contract checks
```

검사 함수는 `assert`가 아니라 실패 시 예외를 던지므로 Release/NDEBUG에서도 실행된다. 11회 조건 평가에는 반복 sweep이 포함되어 있고 Debug·Release·UBSan 재실행을 독립 검사의 수로 더하지 않는다.

## 코드 읽기와 복습

픽셀 반지름 51,49,51,49,44,46,54,56의 순서로 카메라를 움직였다고 가정한다. 단일 threshold는 여러 번 떨지만 45/55 히스테리시스는 44에서 낮은 LOD로, 56에서 높은 LOD로 총 2번만 전환한다.

sphere N개와 고정 6평면이면 O(N), 조기 탈출로 실제 검사 횟수는 달라진다. LOD 결정은 물체당 O(1), 이전 LOD 저장 O(N). 계층적 BVH나 GPU-driven indirect draw는 이 예제에 포함되지 않는다.

실습 확장: 입력 한 조건을 바꾸기 전에 기대 값을 종이에 적고, 기존 경계 테스트를 남긴 채 새 조건을 추가한다. 검사를 지우거나 허용 오차를 키워 실패를 숨기지 않는다. 본문 #review의 여섯 질문을 코드 실행 전후에 비교한다.

## 확인한 범위

2026-10-26 Linux x86_64, GCC 14.2.0, CMake 3.31.6에서 Debug/Release 각각 구성·빌드·CTest 1/1 및 실행 통과. GCC UndefinedBehaviorSanitizer 실행도 통과했다. 실제 로그와 SHA-256은 교재의 `knowledge/shared/graphics-tech-10-lab/verification.json`에 있다.

Windows/MSVC, GPU 셰이더 컴파일·실행, 실제 영상 품질, 디스플레이 색 정확성, GPU 시간은 검증하지 않았다. 그림은 자체 제작 개념도이지 GPU 캡처가 아니다.
