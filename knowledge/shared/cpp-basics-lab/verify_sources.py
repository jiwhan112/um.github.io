#!/usr/bin/env python3
"""원본 첨부의 바이트와 01~05 한국어 주석판의 전처리 전 C++ 토큰을 검사한다.

매크로를 확장하지 않으므로 __LINE__ 진단은 각 파일의 실제 줄을 따른다.
토큰 검사는 컴파일·실행과 별개이며 일반 C++ 의미 동등성의 증명은 아니다.
"""
import argparse
import hashlib
import json
import re
import sys
from dataclasses import dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parent
ATTACHMENT_SHA256 = "c48d34d0e978b43f3e1b81d86e1e902052e6c5f8536c6b372c2fae8a4d2ed761"
SOURCE_REFERENCE = "notion-file-block://5fb8e4e7-6ccb-4edd-889f-bba260b4392b/2d74b3e5-ff5f-4dd0-9fff-4d3559528daf?space_id=815d1335-3b56-43a7-903b-d7addf097c9f&name=cpp_basics_lab_sources_v1.md"
EXPECTED_CHECKS = {"types": 15, "flow": 15, "memory": 19, "containers": 49, "buffer": 104, "inventory": 19}
# 정적 호출 지점 수와 루프 반복을 포함하는 런타임 횟수를 분리한다.
EXPECTED_CHECK_SITES = {"types.cpp": 15, "flow.cpp": 15, "memory.cpp": 19,
                        "containers.cpp": 23, "buffer.cpp": 5,
                        "inventory.hpp": 0, "inventory.cpp": 0, "inventory_main.cpp": 19}
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

def cpp_tokens(source, filename):
    # 현재 주석판 비교 대상에는 줄 연결이 없다. 새로운 전처리 구조는 별도 검토한다.
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


def file_info(data):
    return {"bytes": len(data), "sha256": hashlib.sha256(data).hexdigest()}


def verify():
    repo = ROOT.parents[2]
    captures = {}
    capture_pairs = [(repo / ("knowledge/cpp-basics-" + number) / "source-notion.md",
                      repo / ("knowledge/cpp-basics-" + number) / "source-notion-capture.json")
                     for number in ("01", "02", "03", "04", "05")]
    capture_pairs.append((ROOT / "source-notion-index.md", ROOT / "source-notion-index-capture.json"))
    for source, capture in capture_pairs:
        data = source.read_bytes()
        record = json.loads(capture.read_text(encoding="utf-8"))
        if len(data) != record["source_bytes"] or hashlib.sha256(data).hexdigest() != record["sha256"]:
            raise ValueError(str(source.relative_to(repo)) + ": capture 원장의 바이트/해시 불일치")
        captures[str(source.relative_to(repo))] = {**file_info(data), "capture": str(capture.relative_to(repo)), "matches_capture": True}
    attachment = (ROOT / "source-attachment.md").read_bytes()
    if hashlib.sha256(attachment).hexdigest() != ATTACHMENT_SHA256:
        raise ValueError("보존 첨부 SHA-256이 다릅니다")
    pattern = rb"^#{2,3} ((?:src|negative|diagnostics)/[A-Za-z0-9_.]+|CMakeLists[.]txt)\n\n?```(?:cpp|cmake)\n(.*?)^```[ \t]*(?:\n|$)"
    blocks = re.findall(pattern, attachment, re.MULTILINE | re.DOTALL)
    if len(blocks) != 16 or len({name for name, _ in blocks}) != 16:
        raise ValueError("원문에서 서로 다른 16개 파일 블록을 찾아야 합니다")
    originals = {}
    for name, body in blocks:
        relative = name.decode("ascii")
        actual = (ROOT / "original" / relative).read_bytes()
        if actual != body:
            raise ValueError(relative + ": 원문 블록과 바이트 불일치")
        originals[relative] = file_info(actual)
    annotations = {}
    for name, expected_sites in EXPECTED_CHECK_SITES.items():
        original = (ROOT / "original/src" / name).read_text(encoding="utf-8")
        annotated_bytes = (ROOT / "ko" / name).read_bytes()
        annotated = annotated_bytes.decode("utf-8")
        if directive_lines(original) != directive_lines(annotated):
            raise ValueError(name + ": 전처리 지시문 불일치")
        left = cpp_tokens(original, "original/src/" + name)
        right = cpp_tokens(annotated, "ko/" + name)
        for index in range(max(len(left), len(right))):
            a = left[index] if index < len(left) else None
            b = right[index] if index < len(right) else None
            if a is None or b is None or a.value != b.value:
                raise ValueError("{} token {}: {} != {}".format(name, index + 1, a, b))
        check_calls = sum(token.value == "CHECK" for token in left)
        if check_calls != expected_sites:
            raise ValueError(name + ": CHECK 호출 지점 수가 원문과 다릅니다")
        annotations["ko/" + name] = {
            **file_info(annotated_bytes),
            "original": "original/src/" + name,
            "cpp_tokens_equal_before_macro_expansion": True,
            "cpp_token_count": len(left),
            "directives_exact": True,
            "check_call_sites": check_calls,
            "static_assert_call_sites": sum(token.value == "static_assert" for token in left),
        }
    return {
        "schema_version": 1,
        "status": "pass",
        "source_reference": SOURCE_REFERENCE,
        "notion_page_captures": captures,
        "attachment": {"path": "source-attachment.md", **file_info(attachment)},
        "original_code_blocks_exact": True,
        "original_file_count": 16,
        "original_files": originals,
        "annotated_files": annotations,
        "original_runtime_checks_recorded_in_attachment": EXPECTED_CHECKS,
        "original_runtime_check_invocations_total_including_loop_iterations": 221,
        "first_three_lessons_check_invocations": 49,
        "lessons_04_05_check_invocations": 172,
        "annotated_source_file_count": 8,
        "annotated_program_count": 6,
        "annotation_checks_repeat_original_conditions": True,
        "counting_note": "221 is the original pack's runtime check count including loop iterations: lessons 01-03 account for 49 and lessons 04-05 for 172. Static CHECK call sites are recorded separately. Annotated and compiler reruns do not add independent conditions.",
        "not_recovered": ["historical ZIP attachment", "historical raw build and execution logs"],
        "limitations": ["This is source fidelity verification, not a fresh compile/run result.", "__LINE__ diagnostics follow each source file's actual lines.", "The preserved pack requires C++20; no C++11 compatibility claim."],
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--write-manifest", action="store_true", help="동등성을 확인한 뒤 현재 원장을 기록")
    args = parser.parse_args()
    try:
        result = verify()
        manifest = ROOT / "source-integrity.json"
        if args.write_manifest:
            manifest.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        elif json.loads(manifest.read_text(encoding="utf-8")) != result:
            raise ValueError("source-integrity.json과 현재 검증 결과가 다릅니다")
    except (ValueError, OSError, UnicodeError) as exc:
        print("FAIL: " + str(exc), file=sys.stderr)
        return 1
    if args.json:
        print(json.dumps(result, ensure_ascii=False, indent=2))
    else:
        print("PASS: attachment SHA-256; 16 exact original code blocks")
        for name, info in result["annotated_files"].items():
            print("PASS: {}: {} identical tokens, {} CHECK call sites".format(name, info["cpp_token_count"], info["check_call_sites"]))
        print("NOTE: compilation and execution results are recorded separately")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
