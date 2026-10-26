# 자료구조·알고리즘 05 · 해시 테이블·충돌·동등성

해시와 키 동등성의 역할을 분리하고 평균 비용과 최악 비용을 구별한다.

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
collision keys=1,5,9; 1500 operations against ordered-map oracle
PASS: 80261 contract checks
```

표준입력·인터넷·외부 데이터 파일은 사용하지 않습니다. Check 함수는 Release에서도 유지되며 실패 시 예외로 비정상 종료합니다. 호출 횟수는 반복문을 포함하므로 서로 독립적인 테스트 사례 개수가 아닙니다.

## 코드 읽기와 계약

리소스 ID에서 상태를 찾는 테이블은 빠른 조회가 필요하다. ID를 해시값으로 바꿨다고 원래 키 비교를 생략할 수는 없다. 이 예제는 의도적으로 버킷을4개만 두어 충돌이 흔한 환경에서 정확성을 먼저 확인한다.

이 고정4버킷 구현은 최악 O(n) 조회·삽입·삭제, 저장 O(n). 충분한 버킷과 분포 가정 아래 일반 해시 테이블은 기대 O(1)을 목표로 하지만 무조건 상수 보장은 아니다.

주의: 해시값만 비교해 키를 같다고 처리하면 충돌 키의 값이 덮인다. 삭제 후 남은 이웃 키를 검사하지 않는 테스트는 이런 오류를 놓치기 쉽다.

## 실제 검증

2026-10-26 Linux GCC 14.2.0 C++20 Release에서 CMake 구성·빌드·CTest 1/1 성공. 같은 소스를 UBSan으로 별도 빌드해 출력 일치와 진단 없음 확인. Windows/MSVC·Clang·GPU·성능 측정·다중 스레드는 실행하지 않았습니다. SHA-256과 실행 출력은 교재의 verification.json에 있습니다.

## 출처와 저작

개념 확인 참고: https://eel.is/c++draft/unord.req

게임 상황·설명·그림·코드·테스트는 ATLAS NOTE용 독립 작성입니다. 외부 예제나 교재 본문을 복제하지 않았습니다.
