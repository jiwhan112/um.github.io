#!/usr/bin/env python3
"""정상 실행·컴파일 거부·C++11 미지원·의도적 UB 진단을 별도 결과로 기록한다."""
import argparse
import datetime
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parent
COUNTS = {"types": 15, "flow": 15, "memory": 19, "containers": 49, "buffer": 104, "inventory": 19}


def sources(name, annotated=False):
    names = ["inventory.cpp", "inventory_main.cpp"] if name == "inventory" else [name + ".cpp"]
    directory = ROOT / ("ko" if annotated else "original/src")
    return [str(directory / filename) for filename in names]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("mode", choices=["positive-direct", "compare", "negative", "cxx11", "diagnostics"])
    parser.add_argument("--compiler", default="g++")
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--build", type=Path)
    args = parser.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)
    compiler = shutil.which(args.compiler)
    if args.mode != "compare" and not compiler:
        parser.error("컴파일러를 찾지 못했습니다: " + args.compiler)
    report = {
        "mode": args.mode,
        "executed_at_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "status": "running",
        "commands": [],
        "results": [],
        "counting_note": "Compiler and annotated-source reruns repeat the original checks; do not add them as independent conditions.",
    }
    env = os.environ.copy()
    env.update({"ASAN_OPTIONS": "detect_leaks=1:halt_on_error=1", "UBSAN_OPTIONS": "halt_on_error=1:print_stacktrace=1"})

    def run(label, command):
        result = subprocess.run(command, cwd=ROOT, env=env, stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT, timeout=120)
        (args.out / (label + ".txt")).write_bytes(result.stdout)
        report["commands"].append({"label": label, "argv": command, "cwd": str(ROOT),
                                   "exit_code": result.returncode, "raw_output": label + ".txt"})
        return result

    def require(condition, message):
        if not condition:
            raise ValueError(message)

    def positive_result(label, result, expected):
        require(result.returncode == 0, label + ": 정상 프로그램이 실패했습니다")
        counts = re.findall(rb"^PASS ([0-9]+) checks\r?$", result.stdout, re.MULTILINE)
        require(counts == [str(expected).encode()], label + ": 실제 검사 횟수가 다릅니다")
        report["results"].append({"name": label, "outcome": "normal-pass", "runtime_check_invocations": expected})

    try:
        if compiler:
            version = run("compiler-version", [compiler, "--version"])
            require(version.returncode == 0, "컴파일러 버전 조회 실패")
            report["compiler"] = version.stdout.decode("utf-8", errors="replace").splitlines()[0]
        with tempfile.TemporaryDirectory(prefix="atlas-cpp-basics-") as temporary:
            temp = Path(temporary)
            if args.mode == "positive-direct":
                outputs = {}
                tasks = [(name, sources(name), count) for name, count in COUNTS.items()]
                tasks += [(name + "_ko", sources(name, annotated=True), count) for name, count in COUNTS.items()]
                for name, paths, count in tasks:
                    binary = temp / name
                    command = [compiler, "-std=c++20", "-g", "-O0", "-Wall", "-Wextra", "-Wpedantic",
                               "-Wconversion", "-Wshadow", "-Werror", "-I", str(ROOT / "original/src")]
                    result = run(name + "-build", command + paths + ["-o", str(binary)])
                    require(result.returncode == 0, name + ": C++20 빌드 실패")
                    result = run(name + "-run", [str(binary)])
                    positive_result(name, result, count)
                    outputs[name] = result.stdout
                for name in COUNTS:
                    require(outputs[name] == outputs[name + "_ko"], name + ": 원본·주석판 출력 불일치")
                report.update({"original_runtime_check_invocations": 221, "first_three_lessons": 49,
                               "lessons_04_05": 172, "annotation_check_invocations_repeat_original": 221,
                               "all_six_stdout_pairs_equal": True, "stdout_normalization_applied": False,
                               "cmake_ctest_executed": False})
            elif args.mode == "compare":
                require(args.build is not None, "--build 경로가 필요합니다")
                build = args.build.resolve()
                # 두 프로그램 모두 ROOT에서 실행한다. inventory의 cwd 출력도 바이트 그대로 비교한다.
                for name in COUNTS:
                    original = run(name + "-original", [str(build / "original" / name)])
                    annotated = run(name + "-korean", [str(build / (name + "_ko"))])
                    positive_result(name, original, COUNTS[name])
                    positive_result(name + "_ko", annotated, COUNTS[name])
                    require(original.stdout == annotated.stdout, name + ": 원본·주석판 출력 불일치")
                report["all_six_stdout_pairs_equal"] = True
                report["stdout_normalization_applied"] = False
                report["original_runtime_check_invocations"] = 221
                report["annotation_check_invocations_repeat_original"] = 221
            elif args.mode == "negative":
                for source in sorted((ROOT / "original/negative").glob("*.cpp")):
                    result = run(source.stem, [compiler, "-std=c++20", "-pedantic-errors", "-c", str(source),
                                               "-o", str(temp / (source.stem + ".o"))])
                    require(result.returncode != 0 and b"error:" in result.stdout,
                            source.name + ": 예상한 컴파일 거부가 관찰되지 않았습니다")
                    report["results"].append({"name": source.name, "outcome": "expected-compile-rejection",
                                               "compiler_exit_code": result.returncode})
                require(len(report["results"]) == 4, "컴파일 거부 원본은 4개여야 합니다")
            elif args.mode == "cxx11":
                for name in COUNTS:
                    result = run(name, [compiler, "-std=c++11", "-pedantic-errors", "-fsyntax-only",
                                        "-I", str(ROOT / "original/src")] + sources(name))
                    require(result.returncode != 0 and b"error:" in result.stdout,
                            name + ": C++11 예상 거부가 관찰되지 않았습니다. 결과를 검토하세요")
                    report["results"].append({"name": name, "outcome": "cxx11-unsupported-expected-rejection",
                                               "compiler_exit_code": result.returncode})
                report["cxx11_supported"] = False
                report["cxx11_programs_executed"] = False
            else:
                report["sanitizer_environment"] = {name: env[name] for name in ("ASAN_OPTIONS", "UBSAN_OPTIONS")}
                for name, sanitizer, expected in (
                    ("signed_overflow", "undefined", b"signed integer overflow"),
                    ("use_after_free", "address", b"heap-use-after-free"),
                ):
                    binary = temp / name
                    result = run(name + "-build", [compiler, "-std=c++20", "-g", "-O0", "-fsanitize=" + sanitizer,
                                 "-fno-sanitize-recover=all", "-fno-omit-frame-pointer",
                                 str(ROOT / "original/diagnostics" / (name + ".cpp")), "-o", str(binary)])
                    require(result.returncode == 0, name + ": 진단 예제 빌드 실패")
                    result = run(name + "-expected-failure", [str(binary)])
                    require(result.returncode != 0 and expected in result.stdout,
                            name + ": 예상한 sanitizer 진단과 비정상 종료를 모두 확인하지 못했습니다")
                    report["results"].append({"name": name, "outcome": "expected-undefined-behavior-diagnostic",
                                               "expected_diagnostic": expected.decode(), "program_exit_code": result.returncode,
                                               "normal_program_pass": False})
                report["included_in_normal_ctest"] = False
        report["status"] = "pass"
    except (ValueError, OSError, subprocess.SubprocessError) as exc:
        report["status"] = "fail"
        report["error"] = str(exc)
    finally:
        (args.out / "results.json").write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"mode": args.mode, "status": report["status"], "results": report["results"],
                      "error": report.get("error")}, ensure_ascii=False, indent=2))
    return 0 if report["status"] == "pass" else 1


if __name__ == "__main__":
    raise SystemExit(main())
