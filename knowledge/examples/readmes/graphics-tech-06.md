# 그래픽스 기술 06 · Bloom·분리형 Blur·Tone Mapping

## 무엇을 실행하나

작은 영상에서 2차원 필터와 수평·수직 분리 필터가 같은 결과를 만드는지 확인한다. bloom 합성 순서를 바꾸면 왜 결과가 달라지는지 직접 계산한다.

5×5 단일 채널의 비음수 선형 radiance 배열이다. 밝은 성분은 max(value-threshold,0), 커널은 [1/4,1/2,1/4], 경계는 clamp-to-edge다. RGB hue, 여러 mip의 bloom, exposure adaptation, 실제 GPU ping-pong 자원은 제외한다.

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
center HDR=9.75; SDR=0.906977
PASS: 58 contract checks
```

검사 함수는 `assert`가 아니라 실패 시 예외를 던지므로 Release/NDEBUG에서도 실행된다. 58회 조건 평가에는 반복 sweep이 포함되어 있고 Debug·Release·UBSan 재실행을 독립 검사의 수로 더하지 않는다.

## 코드 읽기와 복습

중앙 한 픽셀8 → threshold1을 빼서7 → 수평 중앙가중치1/2와 수직1/2를 곱해 중앙blur1.75 → 원본과 더해9.75 → Tone=9.75/10.75≈0.906977.

W×H 영상, 폭k 필터는 직접2D O(WHk²), 분리형 O(WHk)이다. 임시영상 O(WH)이 필요하다. 이 예제는 작은 고정배열이고 GPU 메모리 트래픽·dispatch overhead를 측정하지 않았다.

실습 확장: 입력 한 조건을 바꾸기 전에 기대 값을 종이에 적고, 기존 경계 테스트를 남긴 채 새 조건을 추가한다. 검사를 지우거나 허용 오차를 키워 실패를 숨기지 않는다. 본문 #review의 여섯 질문을 코드 실행 전후에 비교한다.

## 확인한 범위

2026-10-26 Linux x86_64, GCC 14.2.0, CMake 3.31.6에서 Debug/Release 각각 구성·빌드·CTest 1/1 및 실행 통과. GCC UndefinedBehaviorSanitizer 실행도 통과했다. 실제 로그와 SHA-256은 교재의 `knowledge/shared/graphics-tech-06-lab/verification.json`에 있다.

Windows/MSVC, GPU 셰이더 컴파일·실행, 실제 영상 품질, 디스플레이 색 정확성, GPU 시간은 검증하지 않았다. 그림은 자체 제작 개념도이지 GPU 캡처가 아니다.
