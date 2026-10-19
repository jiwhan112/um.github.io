# DX9 01–12 구현 예제 · 2026-10-19 GitHub CPU 재검증

기준: [Actions #39217190011](https://github.com/jiwhan112/um.github.io/actions/runs/39217190011), GitHub 원본 소스 기준 SHA `a9218eae7ce7e15ffdb211a15cc8fcd86a9121a3`.

**DX9 CPU 전체 프로젝트 8종을 21개 컴파일러/표준/검사기 구성에서 빌드·CTest 실행해 21/21 구성 통과**했습니다. 21은 독립적 테스트 조건 개수가 아니라 **기존 8종 프로젝트를 환경마다 반복한 횟수**입니다. 02A 공유 라이브러리 계약은 CTest 2개, 나머지는 구성당 1개입니다.

| 프로젝트 | 환경 구성 | CI 상태 | 비고 |
|---|---:|---|---|
| DX9 01A COM | 2 | 각 CTest 1/1 통과 | 실제 Windows D3D9 창 제외 |
| DX9 02A DLL | 2 | 각 CTest 2/2 통과 | Linux 공유 라이브러리, Windows DLL 미실행 |
| DX9 03A VB/IB | 2 | 각 CTest 1/1 통과 | C++20 CPU 버퍼 경계 모형 |
| DX9 04A Renderer | 3 | 각 CTest 1/1 통과 | renderer_components.cpp만 빌드; 부록 notion_example.cpp 별도 미검증 |
| DX9 05A Scene | 3 | 각 CTest 1/1 통과 | 시간 단계·렌더 큐/객체 수명 CPU 모형 |
| DX9 06A Texture | 3 | 각 CTest 1/1 통과 | Pitch/인덱스·별칭 검사 CPU 모형 |
| DX9 07B Picking | 3 | 각 CTest 1/1 통과 | NaN·Ray 경계 CPU 검사 |
| DX9 11A Frustum | 3 | 각 CTest 1/1 통과 | Sphere/AABB CPU 가시성 모형 |

## 실제 실행 원시 로그

다음 21개 파일은 GitHub Actions 각 Job의 **decoded raw 콘솔 로그 전문**입니다. 커밋으로 영구 보관하여 Actions의 14일 Artifact 보존 기간에 의존하지 않습니다. 원본 Notion의 과거 로그를 새 검사인 것처럼 재사용하지 않았습니다.

- [01a-clang-11-san](https://github.com/jiwhan112/um.github.io/blob/master/knowledge/verification/dx9/01a-clang-11-san_run39217190011.log) — [job 117943692113](https://github.com/jiwhan112/um.github.io/actions/runs/39217190011/job/117943692113), CTest 1/1
- [01a-gcc-11-native](https://github.com/jiwhan112/um.github.io/blob/master/knowledge/verification/dx9/01a-gcc-11-native_run39217190011.log) — [job 117943692160](https://github.com/jiwhan112/um.github.io/actions/runs/39217190011/job/117943692160), CTest 1/1
- [02a-clang-11-san](https://github.com/jiwhan112/um.github.io/blob/master/knowledge/verification/dx9/02a-clang-11-san_run39217190011.log) — [job 117943692288](https://github.com/jiwhan112/um.github.io/actions/runs/39217190011/job/117943692288), CTest 2/2
- [02a-gcc-11-native](https://github.com/jiwhan112/um.github.io/blob/master/knowledge/verification/dx9/02a-gcc-11-native_run39217190011.log) — [job 117943692319](https://github.com/jiwhan112/um.github.io/actions/runs/39217190011/job/117943692319), CTest 2/2
- [03a-clang-20-san](https://github.com/jiwhan112/um.github.io/blob/master/knowledge/verification/dx9/03a-clang-20-san_run39217190011.log) — [job 117943692118](https://github.com/jiwhan112/um.github.io/actions/runs/39217190011/job/117943692118), CTest 1/1
- [03a-gcc-20-native](https://github.com/jiwhan112/um.github.io/blob/master/knowledge/verification/dx9/03a-gcc-20-native_run39217190011.log) — [job 117943692133](https://github.com/jiwhan112/um.github.io/actions/runs/39217190011/job/117943692133), CTest 1/1
- [04a-clang-20-san](https://github.com/jiwhan112/um.github.io/blob/master/knowledge/verification/dx9/04a-clang-20-san_run39217190011.log) — [job 117943692342](https://github.com/jiwhan112/um.github.io/actions/runs/39217190011/job/117943692342), CTest 1/1
- [04a-gcc-11-native](https://github.com/jiwhan112/um.github.io/blob/master/knowledge/verification/dx9/04a-gcc-11-native_run39217190011.log) — [job 117943692170](https://github.com/jiwhan112/um.github.io/actions/runs/39217190011/job/117943692170), CTest 1/1
- [04a-gcc-20-native](https://github.com/jiwhan112/um.github.io/blob/master/knowledge/verification/dx9/04a-gcc-20-native_run39217190011.log) — [job 117943692054](https://github.com/jiwhan112/um.github.io/actions/runs/39217190011/job/117943692054), CTest 1/1
- [05a-clang-20-san](https://github.com/jiwhan112/um.github.io/blob/master/knowledge/verification/dx9/05a-clang-20-san_run39217190011.log) — [job 117943692412](https://github.com/jiwhan112/um.github.io/actions/runs/39217190011/job/117943692412), CTest 1/1
- [05a-gcc-11-native](https://github.com/jiwhan112/um.github.io/blob/master/knowledge/verification/dx9/05a-gcc-11-native_run39217190011.log) — [job 117943692243](https://github.com/jiwhan112/um.github.io/actions/runs/39217190011/job/117943692243), CTest 1/1
- [05a-gcc-20-native](https://github.com/jiwhan112/um.github.io/blob/master/knowledge/verification/dx9/05a-gcc-20-native_run39217190011.log) — [job 117943691994](https://github.com/jiwhan112/um.github.io/actions/runs/39217190011/job/117943691994), CTest 1/1
- [06a-clang-20-san](https://github.com/jiwhan112/um.github.io/blob/master/knowledge/verification/dx9/06a-clang-20-san_run39217190011.log) — [job 117943692338](https://github.com/jiwhan112/um.github.io/actions/runs/39217190011/job/117943692338), CTest 1/1
- [06a-gcc-11-native](https://github.com/jiwhan112/um.github.io/blob/master/knowledge/verification/dx9/06a-gcc-11-native_run39217190011.log) — [job 117943692123](https://github.com/jiwhan112/um.github.io/actions/runs/39217190011/job/117943692123), CTest 1/1
- [06a-gcc-20-native](https://github.com/jiwhan112/um.github.io/blob/master/knowledge/verification/dx9/06a-gcc-20-native_run39217190011.log) — [job 117943692286](https://github.com/jiwhan112/um.github.io/actions/runs/39217190011/job/117943692286), CTest 1/1
- [07b-clang-20-san](https://github.com/jiwhan112/um.github.io/blob/master/knowledge/verification/dx9/07b-clang-20-san_run39217190011.log) — [job 117943692181](https://github.com/jiwhan112/um.github.io/actions/runs/39217190011/job/117943692181), CTest 1/1
- [07b-gcc-11-native](https://github.com/jiwhan112/um.github.io/blob/master/knowledge/verification/dx9/07b-gcc-11-native_run39217190011.log) — [job 117943692315](https://github.com/jiwhan112/um.github.io/actions/runs/39217190011/job/117943692315), CTest 1/1
- [07b-gcc-20-native](https://github.com/jiwhan112/um.github.io/blob/master/knowledge/verification/dx9/07b-gcc-20-native_run39217190011.log) — [job 117943692192](https://github.com/jiwhan112/um.github.io/actions/runs/39217190011/job/117943692192), CTest 1/1
- [11a-clang-20-san](https://github.com/jiwhan112/um.github.io/blob/master/knowledge/verification/dx9/11a-clang-20-san_run39217190011.log) — [job 117943692324](https://github.com/jiwhan112/um.github.io/actions/runs/39217190011/job/117943692324), CTest 1/1
- [11a-gcc-11-native](https://github.com/jiwhan112/um.github.io/blob/master/knowledge/verification/dx9/11a-gcc-11-native_run39217190011.log) — [job 117943692216](https://github.com/jiwhan112/um.github.io/actions/runs/39217190011/job/117943692216), CTest 1/1
- [11a-gcc-20-native](https://github.com/jiwhan112/um.github.io/blob/master/knowledge/verification/dx9/11a-gcc-20-native_run39217190011.log) — [job 117943692428](https://github.com/jiwhan112/um.github.io/actions/runs/39217190011/job/117943692428), CTest 1/1

Clang ASan/UBSan 통과는 **실제 실행한 CPU 경로**의 관찰 결과이며 모든 입력의 안전성 증명은 아닙니다. Windows/MSVC, D3D9/GPU 셰이더/FXC, Win32 창·Lost Device/Reset 실기기 작동 및 Unity는 이 런에서 컴파일하거나 실행하지 않았습니다. 별도 코드 예시 중 컴파일 타깃에 포함되지 않은 것은 실행 결과에 포함하지 않습니다.
