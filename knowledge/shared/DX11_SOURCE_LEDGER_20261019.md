# DirectX 11 · 01–08 독립 소스 복구 원장

이번 복구는 연결된 Notion의 네이티브 본문을 직접 읽어 각 파일명 제목 뒤의 fenced code를 파일로 보관했습니다. 각 원본 페이지는 `native-notion-source-NN.md`로 동일하게 별도 저장했습니다. 이 원장은 **Notion 첨부 파일의 바이너리 검증이 아니라 본문 코드 블록에서 원본을 추출한 결과**입니다.

- 01: `shared/dx11-01/`, CPU 계약·Windows 초기화·CMake.
- 02: `shared/dx11-02/`, CPU Resource/View·CMake.
- 03: `shared/dx11-03/`, CPU IA/DrawIndexed·Windows 삼각형·CMake·README.
- 04: `shared/dx11-04/`, CPU Texture/SRV·HLSL·CMake·README.
- 05–08: `shared/dx11-05-08/`, 4개 하위 CPU 소스·CMake, 별도 Windows 텍스처 사각형과 HLSL, 원본 루트 CMake.

- [01A](https://app.notion.com/p/3fba99703f4081958997f12e2fc5a8ab): 18235문자, 원본 코드 파일 3개, 과거 CTest stdout 파일 0개
- [02A](https://app.notion.com/p/3fba99703f40814ebc7fe5250ad8d967): 12830문자, 원본 코드 파일 2개, 과거 CTest stdout 파일 3개
- [03A](https://app.notion.com/p/3fba99703f40819b8a86dd4ff0295fd1): 21312문자, 원본 코드 파일 4개, 과거 CTest stdout 파일 3개
- [04A](https://app.notion.com/p/3fba99703f4081c89295c3aed7177611): 11818문자, 원본 코드 파일 4개, 과거 CTest stdout 파일 3개
- [05A](https://app.notion.com/p/3fba99703f40812ab087df6acc9270f6): 23090문자, 원본 코드 파일 5개, 과거 CTest stdout 파일 3개
- [06A](https://app.notion.com/p/3fba99703f4081ffbf81f558af17fdc6): 15344문자, 원본 코드 파일 2개, 과거 CTest stdout 파일 0개
- [07A](https://app.notion.com/p/3fba99703f4081f2a25dd9874f6fa5cb): 15223문자, 원본 코드 파일 3개, 과거 CTest stdout 파일 0개
- [08A](https://app.notion.com/p/3fba99703f408195b6ebda312ec0c3b6): 14877문자, 원본 코드 파일 2개, 과거 CTest stdout 파일 0개

**기록 구분:** 원본 파일명과 정확한 코드 텍스트는 네이티브 블록에서 가져왔으며, 코드 블록에 존재하지 않은 개행을 임의로 추가하지 않았습니다. GitHub CMake/CTest 새 실행은 CI 워크플로에서 별도 기록합니다. Windows/MSVC/D3D11/GPU/HLSL 컴파일은 수행하지 않았습니다.

01 부모에는 `dx11_init01_full_sources.md` Notion 파일 블록 하나가 더 있습니다. 연결 도구에서 **다운로드 가능한 임시 서명 URL 발급은 성공**했으나 파일 바이트를 직접 검증하거나 첨부 자체를 GitHub로 백업한 것은 아닙니다. 같은 내용을 담은 01A 원본 페이지의 C++/CMake 텍스트를 독립적으로 보존하였으므로 원본 첨부의 바이트 무결성이 입증된 것으로 취급하지 않습니다.
