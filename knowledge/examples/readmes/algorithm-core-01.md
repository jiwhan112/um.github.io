# 자료구조·알고리즘 01 · 비용 모델·복잡도·루프 불변식

코드 길이가 아니라 입력 크기 n과 반복 횟수의 관계를 설명하고, 루프의 중간 상태로 정확성을 증명한다.

## 준비와 실행

C++20 지원 컴파일러와 CMake 3.16 이상이 필요합니다. ZIP을 새 폴더에 풀고 README가 있는 곳에서 다음 명령을 실행합니다. 소스만 제공하며 다운로드가 실행을 시작하지 않습니다.

```sh
cmake -S project -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Linux 직접 실행: `./build/lesson`. Windows 다중 구성 생성기는 `cmake --build build --config Release`, `ctest --test-dir build -C Release --output-on-failure`를 사용합니다. Windows 명령은 안내이며 실제 검증하지 않았습니다.

## 기대 출력

```text
n=8: scan=8 pair=28; n=16: scan=16 pair=120
PASS: 262 contract checks
```

표준입력·인터넷·외부 데이터 파일은 사용하지 않습니다. Check 함수는 Release에서도 유지되며 실패 시 예외로 비정상 종료합니다. 호출 횟수는 반복문을 포함하므로 서로 독립적인 테스트 사례 개수가 아닙니다.

## 코드 읽기와 계약

한 프레임의 피해 이벤트를 합산하는 작업과 모든 플레이어 쌍의 충돌 후보를 만드는 작업은 둘 다 반복문을 사용한다. 그러나 n이 두 배가 될 때 늘어나는 일의 양은 다르다. 프레임 예산을 논할 때 이 차이를 먼저 모델링하고 실제 시간 측정은 나중에 한다.

TotalDamage는 O(n) 시간·O(1) 추가 공간. PairChecks는 O(n²) 시간·O(1) 추가 공간. 입력 벡터 자체 저장 공간은 별도다.

주의: n=0에서 unsigned n−1을 먼저 계산하면 큰 값으로 순환한다. 테스트의 기대 쌍 수는 n==0을 먼저 분리한다. 두 중첩 반복문의 범위가 서로 의존하는지도 확인한다.

## 실제 검증

2026-10-26 Linux GCC 14.2.0 C++20 Release에서 CMake 구성·빌드·CTest 1/1 성공. 같은 소스를 UBSan으로 별도 빌드해 출력 일치와 진단 없음 확인. Windows/MSVC·Clang·GPU·성능 측정·다중 스레드는 실행하지 않았습니다. SHA-256과 실행 출력은 교재의 verification.json에 있습니다.

## 출처와 저작

개념 확인 참고: https://opendatastructures.org/ods-cpp/1_Introduction.html

게임 상황·설명·그림·코드·테스트는 ATLAS NOTE용 독립 작성입니다. 외부 예제나 교재 본문을 복제하지 않았습니다.
