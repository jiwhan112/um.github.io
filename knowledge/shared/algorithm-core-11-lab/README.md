# 자료구조·알고리즘 11 · 다익스트라·A*·휴리스틱 계약

비음수 가중치·일관적 휴리스틱 전제 아래 목표를 꺼냈을 때 최적 비용인 이유를 설명한다.

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
Dijkstra and consistent A*: cost=5; 80 weighted graph oracles
PASS: 3925 contract checks
```

표준입력·인터넷·외부 데이터 파일은 사용하지 않습니다. Check 함수는 Release에서도 유지되며 실패 시 예외로 비정상 종료합니다. 호출 횟수는 반복문을 포함하므로 서로 독립적인 테스트 사례 개수가 아닙니다.

## 코드 읽기와 계약

지역 간 이동 시간이 다르면 최소 간선 수만으로 경로를 고를 수 없다. A*는 이미 이동한 비용g와 목표까지 예상하는 비용h를 합해 후보 순서를 정한다. 예상이 임의로 과장되면 더 빠른 검색처럼 보여도 최적 경로를 놓칠 수 있다.

일관적 h와 비음수 간선에서 유효 확장과 간선 완화는 O(V+E). 중복 후보를 허용한 힙에는 O(E)개 항목이 들어갈 수 있어 시간 O((V+E) log(E+1)), 추가 공간 O(V+E)로 보수적으로 설명한다. 전체 휴리스틱 사전 검증도 O(V+E)다.

주의: 목표를 큐에 넣은 순간 종료하면 아직 더 싼 경로가 나올 수 있다. 유효한 최신 최소 후보로 꺼냈을 때 종료한다. 비용0 간선도 허용하되 더 작은 경우만 갱신해0순환 반복을 막는다.

## 실제 검증

2026-10-26 Linux GCC 14.2.0 C++20 Release에서 CMake 구성·빌드·CTest 1/1 성공. 같은 소스를 UBSan으로 별도 빌드해 출력 일치와 진단 없음 확인. Windows/MSVC·Clang·GPU·성능 측정·다중 스레드는 실행하지 않았습니다. SHA-256과 실행 출력은 교재의 verification.json에 있습니다.

## 출처와 저작

개념 확인 참고: https://www.boost.org/doc/libs/latest/libs/graph/doc/html/graph/algorithms/shortest_paths/astar_search.html

게임 상황·설명·그림·코드·테스트는 ATLAS NOTE용 독립 작성입니다. 외부 예제나 교재 본문을 복제하지 않았습니다.
