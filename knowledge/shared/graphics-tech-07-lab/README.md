# 그래픽스 기술 07 · Temporal AA·재투영·History 거절

## 무엇을 실행하나

motion vector의 방향과 단위를 먼저 정의하고 재투영한 history를 검증한 뒤 혼합한다. ghosting 감소와 정보 손실의 trade-off를 수치 예제로 설명한다.

단일 픽셀의 스칼라 선형 색과 이웃3개의 범위만 사용한다. motion은 previous-current의 비지터 pixel 변위, UV는[0,1), 깊이는 같은 이전 view 공간의 양수 거리다. previous depth에 비교할 expectedPreviousDepth는 상위 변환에서 준비되었다고 가정한다. 실제 지터·행렬 재투영·image sampling·upscaling은 구현하지 않는다.

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
accepted history=0.59; camera cuts and disocclusions reset
PASS: 11 contract checks
```

검사 함수는 `assert`가 아니라 실패 시 예외를 던지므로 Release/NDEBUG에서도 실행된다. 11회 조건 평가에는 반복 sweep이 포함되어 있고 Debug·Release·UBSan 재실행을 독립 검사의 수로 더하지 않는다.

## 코드 읽기와 복습

current=0.5, neighborhood=[0.4,0.5,0.6], history=100 → clamp된 history0.6 → 0.1×0.5+0.9×0.6=0.59. history depth9와 expected depth10의 차이가0.01보다 크면 누적하지 않고0.5를 반환한다.

고정 이웃3개라 픽셀당O(1), K개 neighborhood면O(K). 전체영상과 history는O(WH)저장공간이 필요하지만 본 실습은 단일 픽셀만 계산한다. 실시간 성능·영상 복원 정확성은 미측정이다.

실습 확장: 입력 한 조건을 바꾸기 전에 기대 값을 종이에 적고, 기존 경계 테스트를 남긴 채 새 조건을 추가한다. 검사를 지우거나 허용 오차를 키워 실패를 숨기지 않는다. 본문 #review의 여섯 질문을 코드 실행 전후에 비교한다.

## 확인한 범위

2026-10-26 Linux x86_64, GCC 14.2.0, CMake 3.31.6에서 Debug/Release 각각 구성·빌드·CTest 1/1 및 실행 통과. GCC UndefinedBehaviorSanitizer 실행도 통과했다. 실제 로그와 SHA-256은 교재의 `knowledge/shared/graphics-tech-07-lab/verification.json`에 있다.

Windows/MSVC, GPU 셰이더 컴파일·실행, 실제 영상 품질, 디스플레이 색 정확성, GPU 시간은 검증하지 않았다. 그림은 자체 제작 개념도이지 GPU 캡처가 아니다.
