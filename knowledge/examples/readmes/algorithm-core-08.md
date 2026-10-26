# 자료구조·알고리즘 08 · 정렬·안정성·비교자 계약

정렬 키와 원래 도착 순서를 분리하고 엄격 약순서가 필요한 이유를 설명한다.

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
stable merge sort matches 128 duplicate-rich input lengths
PASS: 8506 contract checks
```

표준입력·인터넷·외부 데이터 파일은 사용하지 않습니다. Check 함수는 Release에서도 유지되며 실패 시 예외로 비정상 종료합니다. 호출 횟수는 반복문을 포함하므로 서로 독립적인 테스트 사례 개수가 아닙니다.

## 코드 읽기와 계약

프레임 이벤트를 우선순위로 정렬하되 같은 우선순위에서는 입력 순서를 유지해야 한다면 안정성이 필요하다. 정렬 결과의 키만 비교하는 테스트는 동점 이벤트가 뒤섞인 버그를 발견하지 못한다.

병합 정렬 시간 O(n log n), 임시 배열 O(n), 재귀 깊이 O(log n). 각 병합에서 임시 배열을 새로 만들지 않고 한 번 확보해 재사용한다.

주의: 동점에서 오른쪽을 먼저 고르면 키는 정렬돼도 도착 순서가 뒤집힐 수 있다. 정렬된 값만 검사하지 말고 원래 ID도 검사한다.

## 실제 검증

2026-10-26 Linux GCC 14.2.0 C++20 Release에서 CMake 구성·빌드·CTest 1/1 성공. 같은 소스를 UBSan으로 별도 빌드해 출력 일치와 진단 없음 확인. Windows/MSVC·Clang·GPU·성능 측정·다중 스레드는 실행하지 않았습니다. SHA-256과 실행 출력은 교재의 verification.json에 있습니다.

## 출처와 저작

개념 확인 참고: https://eel.is/c++draft/alg.sorting

게임 상황·설명·그림·코드·테스트는 ATLAS NOTE용 독립 작성입니다. 외부 예제나 교재 본문을 복제하지 않았습니다.
