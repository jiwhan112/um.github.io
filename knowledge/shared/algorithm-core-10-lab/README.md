# 자료구조·알고리즘 10 · 그래프·BFS·DFS

발견 표시 시점과 방문 순서의 의미를 구분하고 순환·분리 그래프를 처리한다.

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
BFS shortest edge counts and DFS reachability match 100 graph oracles
PASS: 12802 contract checks
```

표준입력·인터넷·외부 데이터 파일은 사용하지 않습니다. Check 함수는 Release에서도 유지되며 실패 시 예외로 비정상 종료합니다. 호출 횟수는 반복문을 포함하므로 서로 독립적인 테스트 사례 개수가 아닙니다.

## 코드 읽기와 계약

게임 지역 연결망에서 어느 지역으로 이동 가능한지와 최소 몇 번의 지역 전환이 필요한지는 서로 다른 질문이다. 연결이 같은 비용일 때 BFS 거리는 최소 간선 수이고 DFS는 도달 여부를 조사하는 데 유용하다.

인접 리스트 검증·BFS·DFS 각각 O(V+E), 거리/방문/큐 또는 스택 O(V). 그래프 자체 O(V+E). 테스트용 모든 쌍 오라클은 O(V³)로 별도다.

주의: 간선마다 이동 시간이 다르면 BFS의 최소 간선 경로가 최소 시간 경로가 아니다. DFS의 첫 발견 경로도 일반적으로 최단이 아니다.

## 실제 검증

2026-10-26 Linux GCC 14.2.0 C++20 Release에서 CMake 구성·빌드·CTest 1/1 성공. 같은 소스를 UBSan으로 별도 빌드해 출력 일치와 진단 없음 확인. Windows/MSVC·Clang·GPU·성능 측정·다중 스레드는 실행하지 않았습니다. SHA-256과 실행 출력은 교재의 verification.json에 있습니다.

## 출처와 저작

개념 확인 참고: https://opendatastructures.org/ods-cpp/12_Graphs.html

게임 상황·설명·그림·코드·테스트는 ATLAS NOTE용 독립 작성입니다. 외부 예제나 교재 본문을 복제하지 않았습니다.
