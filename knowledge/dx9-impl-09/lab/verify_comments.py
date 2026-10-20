#!/usr/bin/env python3
"""09C 보존 원문과 한국어 주석판을 검사한다. 외부 Python 패키지는 필요 없다.

주석·공백을 제외한 전처리 전 토큰을 비교한다. 문자열 안의 //, /* 등을
주석으로 지우지 않으며, + +와 ++도 서로 다른 토큰으로 취급한다.
이 검사는 이 예제의 소스 보존 검사이며 일반적인 C++ 의미 동등성 증명은 아니다.
특히 __LINE__을 확장한 실패 진단의 줄번호는 두 파일에서 달라진다.
"""

import argparse
import hashlib
import json
import re
import sys
from dataclasses import dataclass
from pathlib import Path


ROOT = Path(__file__).resolve().parent
ATTACHMENT_SHA256 = "056fd682246d8f18a7bf599b2d8f165796959e98cbb43b4dce48aa258d42c668"
ORIGINAL_SHA256 = {
    "original/main.cpp": "65414be51c51a6b9fb22f0cf07c296c392b024913f83b241234dc266c53ea12a",
    "original/CMakeLists.txt": "3099695963c1324d57aeea455f884927785ab89bdfefbfa97f323307e1242c6c",
}
IDENTIFIER = re.compile(r"[A-Za-z_][A-Za-z_0-9]*")
RAW_START = re.compile(r'(?:u8|u|U|L)?R"([^\s()\\]{0,16})\(')
QUOTED_START = re.compile(r"(?:u8|u|U|L)?[\"']")
PUNCTUATORS = sorted(
    (
        "%:%:", "<<=", ">>=", "<=>", "...", "->*", "##", "::", ".*",
        "->", "++", "--", "<<", ">>", "<=", ">=", "==", "!=", "&&",
        "||", "*=", "/=", "%=", "+=", "-=", "&=", "^=", "|=", "<:",
        ":>", "<%", "%>", "%:", "{", "}", "[", "]", "(", ")", ";",
        ":", "?", ".", "~", "!", "+", "-", "*", "/", "%", "^",
        "&", "|", "=", "<", ">", ",", "#",
    ),
    key=len,
    reverse=True,
)


@dataclass(frozen=True)
class Token:
    value: str
    line: int
    column: int


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def fenced_block(data, language):
    pattern = rb"^```" + language.encode("ascii") + rb"\r?\n(.*?)^```[ \t]*(?:\r?\n|$)"
    matches = re.findall(pattern, data, re.MULTILINE | re.DOTALL)
    if len(matches) != 1:
        raise ValueError("원문에 {} 코드 블록이 정확히 하나 있어야 합니다".format(language))
    # 마지막 개행과 빈 줄까지 포함한다. strip() 또는 개행 정규화를 사용하지 않는다.
    return matches[0]


def cpp_tokens(source, filename):
    # 현재 두 소스에는 줄 연결이 없다. 새로운 전처리 구조는 묵인하지 않고 별도 검토를 요구한다.
    if "\\\n" in source or "\\\r\n" in source:
        raise ValueError("{}: 줄 연결을 추가하면 검사기 지원 범위를 다시 검토해야 합니다".format(filename))
    tokens = []
    i = 0
    line = 1
    column = 1
    while i < len(source):
        start = i
        emit = True
        if source[i].isspace():
            while i < len(source) and source[i].isspace():
                i += 1
            emit = False
        elif source.startswith("//", i):
            end = source.find("\n", i + 2)
            i = len(source) if end == -1 else end
            emit = False
        elif source.startswith("/*", i):
            end = source.find("*/", i + 2)
            if end == -1:
                raise ValueError("{}:{}: 닫히지 않은 블록 주석".format(filename, line))
            i = end + 2
            emit = False
        else:
            raw = RAW_START.match(source, i)
            quoted = QUOTED_START.match(source, i)
            identifier = IDENTIFIER.match(source, i)
            if raw:
                terminator = ")" + raw.group(1) + '"'
                end = source.find(terminator, raw.end())
                if end == -1:
                    raise ValueError("{}:{}: 닫히지 않은 raw 문자열".format(filename, line))
                i = end + len(terminator)
                suffix = IDENTIFIER.match(source, i)
                if suffix:
                    i = suffix.end()
            elif quoted:
                quote = source[quoted.end() - 1]
                i = quoted.end()
                while i < len(source) and source[i] != quote:
                    if source[i] in "\r\n":
                        raise ValueError("{}:{}: 문자열/문자 리터럴 안의 개행".format(filename, line))
                    i += 2 if source[i] == "\\" else 1
                if i >= len(source):
                    raise ValueError("{}:{}: 닫히지 않은 문자열/문자 리터럴".format(filename, line))
                i += 1
                suffix = IDENTIFIER.match(source, i)
                if suffix:
                    i = suffix.end()
            elif identifier:
                i = identifier.end()
            elif source[i].isdigit() or (
                source[i] == "." and i + 1 < len(source) and source[i + 1].isdigit()
            ):
                # 전처리 숫자를 최대 길이로 읽어 1e-9와 1 - 9, .25와 0.25도 구별한다.
                i += 1
                while i < len(source):
                    char = source[i]
                    if char.isascii() and (char.isalnum() or char in "_.'"):
                        i += 1
                    elif char in "+-" and source[i - 1] in "eEpP":
                        i += 1
                    else:
                        break
            else:
                punctuator = next((p for p in PUNCTUATORS if source.startswith(p, i)), None)
                if punctuator is None:
                    raise ValueError(
                        "{}:{}:{}: 지원하지 않는 코드 문자 {!r}".format(filename, line, column, source[i])
                    )
                i += len(punctuator)
        text = source[start:i]
        if emit:
            tokens.append(Token(text, line, column))
        if "\n" in text:
            line += text.count("\n")
            column = len(text.rsplit("\n", 1)[1]) + 1
        else:
            column += len(text)
    return tokens


def directive_lines(source):
    # 전처리 지시문의 줄 경계와 define 이름-괄호 사이 공백도 그대로 보존한다.
    # 일반 소스의 줄바꿈과 달리 이 둘은 매크로의 의미를 바꿀 수 있다.
    return [line.strip() for line in source.splitlines() if line.lstrip().startswith("#")]


def verify():
    attachment = (ROOT / "source-09c.md").read_bytes()
    if sha256(attachment) != ATTACHMENT_SHA256:
        raise ValueError("source-09c.md의 보존 SHA-256이 다릅니다")
    files = {"source-09c.md": {"bytes": len(attachment), "sha256": sha256(attachment)}}
    for filename, language in (("original/main.cpp", "cpp"), ("original/CMakeLists.txt", "cmake")):
        actual = (ROOT / filename).read_bytes()
        if actual != fenced_block(attachment, language):
            raise ValueError("{}가 원문 코드 블록의 바이트와 다릅니다".format(filename))
        if sha256(actual) != ORIGINAL_SHA256[filename]:
            raise ValueError("{}의 보존 SHA-256이 다릅니다".format(filename))
        files[filename] = {"bytes": len(actual), "sha256": sha256(actual)}

    original = (ROOT / "original/main.cpp").read_bytes().decode("utf-8")
    annotated_bytes = (ROOT / "main.ko.cpp").read_bytes()
    annotated = annotated_bytes.decode("utf-8")
    if directive_lines(original) != directive_lines(annotated):
        raise ValueError("전처리 지시문 또는 매크로 정의가 달라졌습니다")
    left = cpp_tokens(original, "original/main.cpp")
    right = cpp_tokens(annotated, "main.ko.cpp")
    for index in range(max(len(left), len(right))):
        a = left[index] if index < len(left) else None
        b = right[index] if index < len(right) else None
        if a is None or b is None or a.value != b.value:
            raise ValueError("토큰 {} 불일치: 원본 {} / 주석판 {}".format(index + 1, a, b))
    checks = sum(
        token.value == "CHECK" and index > 0 and left[index - 1].value != "define"
        and index + 1 < len(left) and left[index + 1].value == "("
        for index, token in enumerate(left)
    )
    if checks != 53:
        raise ValueError("원본 CHECK 호출은 53개여야 합니다: {}개".format(checks))
    files["main.ko.cpp"] = {"bytes": len(annotated_bytes), "sha256": sha256(annotated_bytes)}
    return {
        "status": "pass",
        "attachment_exact": True,
        "original_code_blocks_exact": True,
        "preprocessing_directives_exact": True,
        "cpp_tokens_equal_before_macro_expansion": True,
        "cpp_token_count": len(left),
        "check_invocations_per_executable": checks,
        "independent_check_conditions": 53,
        "files": files,
        "limitation": "__LINE__ expands to each file's actual line. Compilation and execution are separate checks.",
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--json", action="store_true", help="검사 결과와 현재 파일 해시를 JSON으로 출력")
    args = parser.parse_args()
    try:
        result = verify()
    except (ValueError, OSError, UnicodeError) as exc:
        print("FAIL: {}".format(exc), file=sys.stderr)
        return 1
    if args.json:
        print(json.dumps(result, ensure_ascii=False, indent=2))
    else:
        print("PASS: 09C attachment SHA-256 and original C++/CMake bytes")
        print("PASS: preprocessing directives and {} C++ tokens match".format(result["cpp_token_count"]))
        print("PASS: 53 CHECK invocations; repeated builds do not add independent conditions")
        print("NOTE: __LINE__ uses each source file's actual line; compilation is checked separately")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
