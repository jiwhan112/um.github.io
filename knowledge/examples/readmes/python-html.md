# 오프라인 HTML 카드 추출 · 예제 다운로드

## 준비와 실행

Python 3.9 이상과 Beautiful Soup 4가 필요합니다. 원문에 기록된 검증 버전은 Python 3.13.5 / beautifulsoup4 4.14.3입니다. 아래는 POSIX 셸 명령이며 패키지 설치에는 인터넷 또는 준비된 패키지 저장소가 필요할 수 있습니다.

```sh
cd project
python3 -m venv .venv
. .venv/bin/activate
python -m pip install beautifulsoup4==4.14.3
python main.py --selftest
python main.py
```

`--selftest`는 unittest **10개 통과, OK**가 정상입니다(실행 시간은 달라집니다). 인자 없이 실행하면 JSON 배열에 다음 두 객체를 출력합니다.

```json
[
  {"title": "First", "url": "https://news.example/post/1", "author": "Ada"},
  {"title": "Second", "url": "https://news.example/post/2", "author": "(unknown)"}
]
```

HTML fixture는 main.py 안에 들어 있습니다. 입력 파일·실제 뉴스 사이트·API 키는 필요 없고 프로그램 자체는 네트워크를 호출하지 않습니다. 출력 URL은 테스트 데이터이지 실제로 접속한 결과가 아닙니다. 이 예제는 웹 크롤러나 임의 HTML의 보안 정화기가 아닙니다. venv와 설치 패키지는 ZIP에 포함하지 않습니다. 새 의존성 설치·실행은 이 패키징 작업에서 수행하지 않았습니다.


## 원본과 검증 경계

이 팩은 기존 공개 교재의 실제 전체 소스만 묶습니다. 기존 코드 바이트는 바꾸지 않았습니다. ZIP 루트의 이 README만 다운로드용 안내입니다. 외부 페이지·원본 첨부·검증 로그를 실행 의존성으로 넣지 않았습니다.

패키징 단계에서는 CPU 검사를 재실행하지 않았습니다. 기존 실행 기록과 소스 해시 대조는 신규 빌드 성공이나 사용자의 플랫폼 실행을 뜻하지 않습니다. 압축 해제 뒤 위 명령을 직접 실행한 결과와 구분하세요. Windows/MSVC·GPU·성능에 대한 신규 검증은 없습니다.

교재: https://jiwhan112.github.io/um.github.io/knowledge/python-html/
