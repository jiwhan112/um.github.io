# 그래픽스 기술 02 · 법선·역전치·TBN·노멀 맵

## 무엇을 실행하나

모델의 비균일 scale 뒤에도 표면 법선과 접선이 수직이어야 한다는 불변식을 코드로 확인한다. 노멀 맵의 세 성분이 어떤 기저에서 표현되는지와 미러 UV의 handedness를 설명한다.

오른손 3차원 벡터와 열벡터 관점으로 설명한다. 실습의 ScaleNormal은 대각 스케일만 처리하며 일반 4×4 행렬 역행렬 구현이 아니다. 입력 법선·접선은 유한하고 길이가 0이 아니어야 한다. TBN의 tangent는 법선과 평행할 수 없다. 노멀 맵 입력은 이미 [-1,1]로 디코딩된 벡터다.

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
correct transformed normal dot tangent=0
PASS: 12 contract checks
```

검사 함수는 `assert`가 아니라 실패 시 예외를 던지므로 Release/NDEBUG에서도 실행된다. 12회 조건 평가에는 반복 sweep이 포함되어 있고 Debug·Release·UBSan 재실행을 독립 검사의 수로 더하지 않는다.

## 코드 읽기와 복습

t=(1,-1,0)/√2, n=(1,1,0)/√2를 사용한다. Mt=(√2,-1/√2,0). 올바른 법선은 normalize(1/(2√2),1/√2,0)이므로 내적은 0이다. 잘못된 normalize(Mn)은 수직 조건을 깨뜨린다.

3차원 벡터와 대각 scale만 처리하므로 각 호출 O(1)이다. mesh의 V개 정점에 적용하면 O(V). 실제 vertex shader 처리량이나 normal-map 대역폭은 측정하지 않았다.

실습 확장: 입력 한 조건을 바꾸기 전에 기대 값을 종이에 적고, 기존 경계 테스트를 남긴 채 새 조건을 추가한다. 검사를 지우거나 허용 오차를 키워 실패를 숨기지 않는다. 본문 #review의 여섯 질문을 코드 실행 전후에 비교한다.

## 확인한 범위

2026-10-26 Linux x86_64, GCC 14.2.0, CMake 3.31.6에서 Debug/Release 각각 구성·빌드·CTest 1/1 및 실행 통과. GCC UndefinedBehaviorSanitizer 실행도 통과했다. 실제 로그와 SHA-256은 교재의 `knowledge/shared/graphics-tech-02-lab/verification.json`에 있다.

Windows/MSVC, GPU 셰이더 컴파일·실행, 실제 영상 품질, 디스플레이 색 정확성, GPU 시간은 검증하지 않았다. 그림은 자체 제작 개념도이지 GPU 캡처가 아니다.
