# DX11 09–12 신규 CPU 빌드·CTest 원시 로그

기록일: 2026-10-19 (KST). [Actions Run #39228648060](https://github.com/jiwhan112/um.github.io/actions/runs/39228648060). 실제로 수행한 GitHub CI 출력 전문을 별도 파일로 저장했습니다.

| 환경 | 09 Assimp-like | 10 Animation | 11 Collider | 12 NavMesh |
| --- | --- | --- | --- | --- |
| GCC C++20 | 1/1 통과, 27 조건 | 1/1 통과, 33 조건 | 1/2 중 #1 통과, 29 조건 | 2/2 중 #2 통과, 27 조건 |
| GCC C++11 | 동일 CPU 모델 통과 | 동일 CPU 모델 통과 | 동일 CPU 모델 통과 | 동일 CPU 모델 통과 |
| Clang C++20 · ASan/UBSan | 동일 CPU 모델 통과 | 동일 CPU 모델 통과 | 동일 CPU 모델 통과 | 동일 CPU 모델 통과 |

- 총 **4개 독립 CPU 프로그램, 고유 논리 조건 116개(27+33+29+27)**입니다. 세 환경에서 같은 조건을 반복한 것이므로 **348개의 다른 조건**으로 중복 집계하지 않습니다.
- 원시 로그: [GCC C++20](gcc20_actions_run39228648060.log) / [GCC C++11](gcc11_actions_run39228648060.log) / [Clang ASan/UBSan](clangsan_actions_run39228648060.log). 실제 GitHub Actions 단계의 Configure·Build·CTest 출력과 도구 버전은 해당 텍스트 원본에 있습니다.
- 원본 Notion 09A/10A의 과거 CTest stdout과 이번 새 GitHub CI 원시는 별도 실행입니다. Notion 11A/12A의 과거 결과는 발췌본/요약이므로 전체 원시 기록으로 오인하지 않습니다.
- Windows/MSVC·Direct3D11 GPU·Assimp SDK·Bone Skinning HLSL·Recast/Detour/Agent Radius는 **이번 CI에서 컴파일·실행하지 않았습니다**.
