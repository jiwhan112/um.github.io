# DX11 13–18 · 최초 독립 GitHub CPU 실행 원본 (2026-10-20)

[GitHub Actions #39351876445](https://github.com/jiwhan112/um.github.io/actions/runs/39351876445)에서 복원된 GitHub 파일을 독립 컴파일하고 CTest 실행한 **실제 GitHub Actions 전체 Job 로그** 세 개를 여기에 보존합니다.

| 환경 | CTest | 실행 논리 조건 |
|---|---|---|
| GCC C++20 | 13 / 13B / 14 / 16 / 17 / 18 각 1/1, 합계 6/6 | 37+21+31+124+49+41 = **303** |
| GCC C++11 | 같은 6개 프로그램 각각 1/1 | 같은 303조건 반복 |
| Clang C++20 ASan/UBSan | 같은 6개 프로그램 각각 1/1 | 같은 303조건 반복 |

- [gcc20 작업 #118397712741](https://github.com/jiwhan112/um.github.io/actions/runs/39351876445/job/118397712741) · [전체 출력 원문](./gcc20_run39351876445.raw.log)
- [gcc11 작업 #118397712659](https://github.com/jiwhan112/um.github.io/actions/runs/39351876445/job/118397712659) · [전체 출력 원문](./gcc11_run39351876445.raw.log)
- [clangsan 작업 #118397712539](https://github.com/jiwhan112/um.github.io/actions/runs/39351876445/job/118397712539) · [전체 출력 원문](./clangsan_run39351876445.raw.log)

이 로그는 편집된 테스트 요약이 아니라 GitHub Actions 콘솔 전체 출력입니다. 기록된 테스트 성공은 Windows/MSVC·D3D11/GPU/HLSL 실기기 실행이 아니며 역대 원본 소스의 SHA 동일성을 입증하지 않습니다.
