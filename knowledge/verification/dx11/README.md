# DX11 01–08 · GitHub 독립 CPU 재검증 (2026-10-19)

이 기록은 과거 Notion의 검사 로그가 아니라 **새로 복구한 GitHub C++/CMake 파일에 대해 GitHub Actions가 실제 실행한 전체 작업 로그**입니다.

[GitHub Actions #39222967585](https://github.com/jiwhan112/um.github.io/actions/runs/39222967585)

| 환경 | CTest 대상 | 결과 | 원시 로그 |
|---|---|---|---|
| gcc20 | 01·02·03·04 각 1/1 + 묶음 05–08 4/4 | 8/8 프로그램 통과 | [직접 출력](./gcc20_github_actions_39222967585.raw.log) |
| gcc11 | 01·02·03·04 각 1/1 + 묶음 05–08 4/4 | 8/8 프로그램 통과 | [직접 출력](./gcc11_github_actions_39222967585.raw.log) |
| clangsan | 01·02·03·04 각 1/1 + 묶음 05–08 4/4 | 8/8 프로그램 통과 | [직접 출력](./clangsan_github_actions_39222967585.raw.log) |

각 환경에서 같은 교육용 CPU 계약 검사를 반복했습니다. 3개 컴파일러/표준 조합을 **24개의 독립 학습 프로그램**으로 중복 집계하지 않습니다. 8개 CPU 프로그램의 고유 조건 수는 Notion 원본에 적힌 01=24, 02=40, 03=32, 04=36, 05=29, 06=30, 07=28, 08=33 (총 252개 조건, 각 환경 반복)입니다. 정확한 실행 문자열·컴파일러 버전·CMake/CTest stdout/stderr는 원시 로그를 참조하십시오.

**Linux 검증만 수행**하였으며 Windows/MSVC·D3D11/GPU·HLSL FXC/DXC 컴파일과 성능 측정·실제 이미지 비교는 검증하지 않았습니다. ASan/UBSan에서 통과하더라도 코드 전체의 UB/메모리 안전성을 증명하지 않습니다.
