#!/usr/bin/env python3
"""ZIP 원본·Markdown 경계·과거 로그·원문 capture·한국어 코드 토큰을 각각 검사한다."""
import argparse
import hashlib
import json
import re
import sys
import zipfile
from dataclasses import dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parent
ZIP_NAME = "cpp_advanced_effective_lab_v1.zip"
ZIP_PREFIX = "cpp_advanced_effective_lab_v1/"
ZIP_SHA256 = "a97a4927e5d3472ad055dbffbad0e14ab72c4c81f6ea13a1d72b771d64d339ba"
MARKDOWN_NAME = "cpp_advanced_effective_sources_v1.md"
MARKDOWN_SHA256 = "62a33518b9ba68cb689ad6b330a1327160f834ee8df9f052f11f34aef7986a4e"
VALIDATION_NAME = "validation-notion-attachment.md"
VALIDATION_SHA256 = "e9f9b5dbba6f631e9a95f4ac426647603eefcfe7d758d85a3541b41ee82bdeff"
VALIDATION_REFERENCE = "notion-file-block://73d8331f-f637-4b17-9d72-49af281d846a/145d187d-b1de-4dbd-9fb8-df133567229d?space_id=815d1335-3b56-43a7-903b-d7addf097c9f&name=cpp_advanced_effective_validation_v1.md"
COUNTS = {"lifetime": 10, "ownership": 13, "polymorphism": 12, "templates": 10,
          "containers": 16, "interfaces": 7, "transaction": 8, "allocation": 7, "pimpl": 6}
ANNOTATED = {"check.hpp": (0, 0), "lifetime.cpp": (7, 3),
             "ownership.cpp": (11, 2), "transaction.cpp": (6, 2),
             "polymorphism.cpp": (10, 2), "templates.cpp": (8, 2),
             "interfaces.cpp": (6, 1), "containers.cpp": (14, 0),
             "allocation.cpp": (5, 2), "pimpl.hpp": (0, 0),
             "pimpl.cpp": (0, 0), "pimpl_main.cpp": (5, 1)}
LESSON_GROUPS = {"01-02": ("lifetime", "ownership", "transaction"),
                "03-04": ("polymorphism", "templates", "interfaces"),
                "05-06": ("containers", "allocation", "pimpl")}

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


def info(data):
    return {"bytes": len(data), "sha256": hashlib.sha256(data).hexdigest()}


def destination(relative):
    if relative.startswith("logs/"):
        return ROOT / "verification/historical-20261013" / relative
    return ROOT / "original" / relative


def verify():
    repo = ROOT.parents[2]
    captures = {}
    pairs = [(ROOT / "source-notion-index.md", ROOT / "source-notion-index-capture.json")]
    pairs += [(repo / ("knowledge/cpp-advanced-" + number + "/source-notion.md"),
               repo / ("knowledge/cpp-advanced-" + number + "/source-notion-capture.json")) for number in ("01", "02", "03", "04", "05", "06")]
    for source, capture in pairs:
        data = source.read_bytes()
        record = json.loads(capture.read_text(encoding="utf-8"))
        if info(data) != {"bytes": record["source_bytes"], "sha256": record["sha256"]}:
            raise ValueError(str(source) + ": capture bytes/hash mismatch")
        captures[str(source.relative_to(repo))] = {**info(data), "capture": str(capture.relative_to(repo)), "matches_capture": True}

    archives = {}
    for name, expected in ((ZIP_NAME, ZIP_SHA256), (MARKDOWN_NAME, MARKDOWN_SHA256), (VALIDATION_NAME, VALIDATION_SHA256)):
        data = (ROOT / "archive" / name).read_bytes()
        if hashlib.sha256(data).hexdigest() != expected:
            raise ValueError(name + ": archived artifact SHA-256 mismatch")
        archives["archive/" + name] = info(data)

    zip_files = {}
    with zipfile.ZipFile(ROOT / "archive" / ZIP_NAME) as archive:
        entries = [entry for entry in archive.infolist() if not entry.is_dir()]
        if len(entries) != 44 or len({entry.filename for entry in entries}) != 44:
            raise ValueError("Expected 44 unique regular files in historical ZIP")
        for entry in entries:
            if not entry.filename.startswith(ZIP_PREFIX):
                raise ValueError("Unexpected ZIP prefix")
            relative = entry.filename[len(ZIP_PREFIX):]
            if Path(relative).is_absolute() or ".." in Path(relative).parts:
                raise ValueError("Unsafe archive path")
            data = archive.read(entry)
            data.decode("utf-8")
            saved = destination(relative)
            if saved.read_bytes() != data:
                raise ValueError(relative + ": ZIP entry differs from preserved file")
            zip_files[relative] = {**info(data), "path": str(saved.relative_to(ROOT)), "zip_entry_exact": True,
                                   "kind": "historical-raw-log" if relative.startswith("logs/") else "preserved-file"}
        markdown = (ROOT / "archive" / MARKDOWN_NAME).read_bytes()
        blocks = re.findall(rb"^## `([^`]+)`\n\n````[^\n]*\n(.*?)^````[ \t]*(?:\n|$)", markdown, re.MULTILINE | re.DOTALL)
        if len(blocks) != 21 or len({name for name, _ in blocks}) != 21:
            raise ValueError("Expected 21 unique source Markdown blocks")
        boundaries = {}
        for name, body in blocks:
            relative = name.decode("utf-8")
            original = archive.read(ZIP_PREFIX + relative)
            # 모든 원본 Markdown 블록은 ZIP 파일 뒤에 LF 한 개를 추가한 형식이다.
            # ZIP과 Markdown을 각각 그대로 보존하고 이 차이를 명시한다.
            if body != original + b"\n":
                raise ValueError(relative + ": unexpected Markdown/ZIP boundary difference")
            boundaries[relative] = {"markdown_block": info(body), "zip_file": info(original),
                                    "relationship": "markdown_block_equals_zip_file_plus_one_LF",
                                    "zip_original_modified": False}
        validation = (ROOT / "archive" / VALIDATION_NAME).read_bytes()
        if ZIP_SHA256.encode() not in validation:
            raise ValueError("Notion validation attachment does not record the archived ZIP SHA")
        negatives = re.findall(rb"^### (negative/[^\n]+)\n```cpp\n(.*?)^```[ \t]*(?:\n|$)", validation, re.MULTILINE | re.DOTALL)
        if len(negatives) != 6:
            raise ValueError("Expected all six negative code blocks in Notion validation attachment")
        for name, body in negatives:
            if body != archive.read(ZIP_PREFIX + name.decode()):
                raise ValueError(name.decode() + ": validation attachment code block mismatch")

    annotations = {}
    for filename, (expected_expect, expected_throws) in ANNOTATED.items():
        original = (ROOT / "original/src" / filename).read_text(encoding="utf-8")
        data = (ROOT / "ko" / filename).read_bytes()
        annotated = data.decode("utf-8")
        a = cpp_tokens(original, "original/src/" + filename)
        b = cpp_tokens(annotated, "ko/" + filename)
        if [token.value for token in a] != [token.value for token in b]:
            raise ValueError(filename + ": C++ token mismatch")
        if directive_lines(original) != directive_lines(annotated):
            raise ValueError(filename + ": preprocessor directive mismatch")
        expect_sites = len(re.findall(r"\bc[.]Expect\s*\(", original))
        throws_sites = len(re.findall(r"\bc[.]Throws\s*<", original))
        if (expect_sites, throws_sites) != (expected_expect, expected_throws):
            raise ValueError(filename + ": test call-site mismatch")
        annotations["ko/" + filename] = {**info(data), "original": "original/src/" + filename,
            "cpp_tokens_equal_before_macro_expansion": True, "cpp_token_count": len(a), "directives_exact": True,
            "expect_call_sites": expect_sites, "throws_call_sites": throws_sites,
            "unconditional_expect_true_sites": len(re.findall(r"\bc[.]Expect\(true,", original)),
            "static_assert_call_sites": sum(token.value == "static_assert" for token in a)}
    return {"schema_version": 1, "status": "pass", "artifacts": archives,
        "provenance": {"zip_and_source_markdown": "Recovered original conversation artifacts; preserved byte-for-byte.",
                       "notion_validation_attachment": VALIDATION_REFERENCE,
                       "zip_sha256_matches_notion_validation_record": True},
        "notion_page_captures": captures, "zip_file_count": 44, "zip_files": zip_files,
        "historical_log_file_count": 22, "markdown_code_block_count": 21, "markdown_zip_boundaries": boundaries,
        "validation_negative_blocks_exact": 6, "annotated_files": annotations,
        "original_runtime_check_invocations": COUNTS,
        "original_runtime_check_invocations_total": 89, "selected_lessons_runtime_check_invocations": 89,
        "selected_lessons": "01-06", "current_batch": "05-06",
        "runtime_check_invocations_by_lesson_batch": {
            batch: sum(COUNTS[name] for name in names) for batch, names in LESSON_GROUPS.items()},
        "current_batch_runtime_check_invocations": 29,
        "annotation_check_invocations_repeat_original": 89,
        "counting_note": "89 counts calls in nine original programs. Lessons 01-02 account for 31 (10+13+8), lessons 03-04 for 29 (12+10+7), and current lessons 05-06 for 29 (16+7+6), totaling 89 across annotated programs. containers has 14 Expect call sites but executes 16 calls because its three-item irreflexivity loop invokes one site three times. ownership includes one Expect(true) following a compile-time guaranteed-elision example. templates has three static_assert sites, interfaces one, ownership two, and pimpl_main one; these are separate from runtime calls. Compiler/annotation reruns add no independent conditions.",
        "limitations": ["Source fidelity and historical logs do not replace a new compile/run.",
                       "Recovered historical ZIP identity is confirmed against the Notion-recorded SHA-256.",
                       "Windows/MSVC, GPU, concurrency, ABI and performance were not validated by this script.",
                       "The pack targets C++20; C++11 compatibility is checked separately with strict compile diagnostics."]}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--write-manifest", action="store_true")
    args = parser.parse_args()
    try:
        result = verify()
        path = ROOT / "source-integrity.json"
        if args.write_manifest:
            path.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        elif json.loads(path.read_text(encoding="utf-8")) != result:
            raise ValueError("source-integrity.json differs from current verification")
    except (ValueError, OSError, UnicodeError, zipfile.BadZipFile, KeyError) as exc:
        print("FAIL: " + str(exc), file=sys.stderr)
        return 1
    if args.json:
        print(json.dumps(result, ensure_ascii=False, indent=2))
    else:
        print("PASS: historical ZIP SHA-256; 44 exact extracted files; 22 historical raw logs")
        print("PASS: 21 Markdown blocks equal ZIP files plus one LF; six Notion negative blocks exact")
        for name, data in result["annotated_files"].items():
            print("PASS: {}: {} identical C++ tokens".format(name, data["cpp_token_count"]))
        print("NOTE: fresh execution recorded separately; 89 whole-pack calls; lessons 01-02: 31, 03-04: 29, current 05-06: 29, annotated total: 89")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
