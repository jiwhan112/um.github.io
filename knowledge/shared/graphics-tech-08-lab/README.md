# 그래픽스 기술 08 · NavMesh Corridor·Portal·Funnel

## 무엇을 실행하나

폴리곤 경로와 실제 이동 waypoint가 서로 다른 결과임을 이해하고, portal 통로의 좌우 경계가 교차할 때 코너를 확정한다. 렌더링 기술과 구별되는 공간·이동 기술로 다룬다.

평면의 y가 위로 증가하며 portal은 진행 방향에서 본 left/right 순서다. 이미 유효한 단순 corridor가 준비되었다고 가정한다. 좌표절댓값≤10^6, portal≤9998개, start/end는 각 끝 폴리곤에 있다. 에이전트는 점이며 반지름 처리·3D volume·off-mesh link·동적 회피·navmesh 생성/A*는 구현하지 않는다.

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
L-corridor corners=3 length=5.84162
PASS: 110 contract checks
```

검사 함수는 `assert`가 아니라 실패 시 예외를 던지므로 Release/NDEBUG에서도 실행된다. 110회 조건 평가에는 반복 sweep이 포함되어 있고 Debug·Release·UBSan 재실행을 독립 검사의 수로 더하지 않는다.

## 코드 읽기와 복습

start(0,0), 첫 portal left(2,1)/right(2,-1), 다음 portal left(2,3)/right(4,3), end(4,4)에서 직선은 첫 portal의 위를 벗어난다. 결과는 (0,0)→(2,1)→(4,4), 길이√5+√13≈5.84162다.

단순 restart형 funnel은 최악O(N²)시간, portal 복사와 결과 O(N)공간이다. 고급 선형 구현과 구별한다. 그래프 탐색 비용과 이동 회피 비용은 포함하지 않는다.

실습 확장: 입력 한 조건을 바꾸기 전에 기대 값을 종이에 적고, 기존 경계 테스트를 남긴 채 새 조건을 추가한다. 검사를 지우거나 허용 오차를 키워 실패를 숨기지 않는다. 본문 #review의 여섯 질문을 코드 실행 전후에 비교한다.

## 확인한 범위

2026-10-26 Linux x86_64, GCC 14.2.0, CMake 3.31.6에서 Debug/Release 각각 구성·빌드·CTest 1/1 및 실행 통과. GCC UndefinedBehaviorSanitizer 실행도 통과했다. 실제 로그와 SHA-256은 교재의 `knowledge/shared/graphics-tech-08-lab/verification.json`에 있다.

Windows/MSVC, GPU 셰이더 컴파일·실행, 실제 영상 품질, 디스플레이 색 정확성, GPU 시간은 검증하지 않았다. 그림은 자체 제작 개념도이지 GPU 캡처가 아니다.
