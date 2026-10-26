# 그래픽스 기술 05 · Cascaded Shadow Map·분할·Texel 안정화

## 무엇을 실행하나

near/far 구간을 여러 cascade로 나누는 이유와 각 shadow texel의 월드 크기를 계산한다. 카메라가 조금 움직일 때 projection 위치를 고정하는 것과 projection 크기를 안정화하는 것이 다름을 설명한다.

카메라 앞의 양수 view depth, 10^-4≤near<far≤10^6를 사용한다. split은 최대16개, lambda∈[0,1]이다. Snap의 extent는 고정된 광원 orthographic 반폭, resolution은 해당 축의 texel 수다. 실제 frustum corner 생성·caster bounds·광원 행렬·GPU 렌더링은 구현하지 않는 산술 실습이다. Snap은 |center|≤10^6, 10^-4≤extent≤10^6, 1≤resolution≤16384의 교육용 수치 범위만 지원한다. 이 범위를 벗어난 유한값도 거절한다.

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
mixed splits: 1 14.5851 30.5249 53.9298 101
PASS: 222 contract checks
```

검사 함수는 `assert`가 아니라 실패 시 예외를 던지므로 Release/NDEBUG에서도 실행된다. 222회 조건 평가에는 반복 sweep이 포함되어 있고 Debug·Release·UBSan 재실행을 독립 검사의 수로 더하지 않는다.

## 코드 읽기와 복습

e=10, R=100이면 texel=0.2. snap(0.01)=0, snap(0.09)=0, snap(0.11)=0.2. 양자화 이동 오차는 반 texel인0.1이하이다. 음수 좌표에서도 floor(x/texel+0.5)를 써 동일한 격자를 유지한다.

분할 생성 O(N) 공간·시간, 이 예제의 cascade 선택은 O(N), snap은 O(1)이다. 작은 고정 N에 대한 선형 탐색을 썼다. 실제 shadow draw call이나 GPU 비용은 별도다.

실습 확장: 입력 한 조건을 바꾸기 전에 기대 값을 종이에 적고, 기존 경계 테스트를 남긴 채 새 조건을 추가한다. 검사를 지우거나 허용 오차를 키워 실패를 숨기지 않는다. 본문 #review의 여섯 질문을 코드 실행 전후에 비교한다.

## 확인한 범위

2026-10-26 Linux x86_64, GCC 14.2.0, CMake 3.31.6에서 Debug/Release 각각 구성·빌드·CTest 1/1 및 실행 통과. GCC UndefinedBehaviorSanitizer 실행도 통과했다. 실제 로그와 SHA-256은 교재의 `knowledge/shared/graphics-tech-05-lab/verification.json`에 있다.

Windows/MSVC, GPU 셰이더 컴파일·실행, 실제 영상 품질, 디스플레이 색 정확성, GPU 시간은 검증하지 않았다. 그림은 자체 제작 개념도이지 GPU 캡처가 아니다.
