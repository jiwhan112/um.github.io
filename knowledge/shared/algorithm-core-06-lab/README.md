# 자료구조·알고리즘 06 · 이진 탐색 트리·정렬 맵

BST 탐색의 올바름을 설명하면서 균형 없는 트리에 O(log n)을 잘못 약속하지 않는다.

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
BST order verified; ascending input creates an unbalanced chain
PASS: 1251 contract checks
```

표준입력·인터넷·외부 데이터 파일은 사용하지 않습니다. Check 함수는 Release에서도 유지되며 실패 시 예외로 비정상 종료합니다. 호출 횟수는 반복문을 포함하므로 서로 독립적인 테스트 사례 개수가 아닙니다.

## 코드 읽기와 계약

스킬 쿨다운 ID를 정렬해 탐색하거나 범위 순회를 하고 싶다면 키 순서가 있는 구조가 유용하다. 이진 탐색 트리라는 이름만으로 균형이 보장되지는 않는다. 이미 정렬된 키를 넣었을 때 어떤 모양이 되는지 관찰한다.

삽입·조회 O(h), 최악 h=n이면 O(n). 중위 순회 O(n), 명시적 스택 O(h), 결과 벡터 O(n). 균형 트리의 h=O(log n)은 이 구현의 보장이 아니다.

주의: root 왼쪽 자식만 작다고 확인해도 그 손자가 root보다 클 수 있다. 지역 비교 몇 개를 전체 BST 불변식의 증명으로 오해하지 않는다.

## 실제 검증

2026-10-26 Linux GCC 14.2.0 C++20 Release에서 CMake 구성·빌드·CTest 1/1 성공. 같은 소스를 UBSan으로 별도 빌드해 출력 일치와 진단 없음 확인. Windows/MSVC·Clang·GPU·성능 측정·다중 스레드는 실행하지 않았습니다. SHA-256과 실행 출력은 교재의 verification.json에 있습니다.

## 출처와 저작

개념 확인 참고: https://opendatastructures.org/ods-cpp/6_Binary_Trees.html

게임 상황·설명·그림·코드·테스트는 ATLAS NOTE용 독립 작성입니다. 외부 예제나 교재 본문을 복제하지 않았습니다.

## 복사·이동 정책과 회귀 검사

SearchTree는 이 교육 구현에서 복사 생성·복사 대입·이동 생성·이동 대입을 모두 명시적으로 금지한다. 저장소만 기본 이동하고 size/head/capacity가 남으면 이동 원본을 정상 객체처럼 사용하는 순간 불변식이 깨진다. 이동 후 상태를 복구하는 구현은 이번 범위에 없으므로 컴파일 단계에서 그런 사용을 거절한다. 객체 자체의 수명은 그대로 사용하고 다른 소유 객체로 옮기는 기능이 필요하면 별도 계약을 설계해야 한다.

복사 생성·복사 대입·이동 생성·이동 대입의 type_traits를 검사하는 static_assert 4개가 있습니다. 이 컴파일 시점 검사는 출력의 런타임 Check 수에 합산하지 않습니다. 수정된 동일 소스로 Release CTest 및 UBSan을 다시 실행했습니다.
