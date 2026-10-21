#!/usr/bin/env python3
"""정상 프로그램, 컴파일 거부, 엄격한 C++11 호환 검사를 구분해 원시 출력을 남긴다."""
import argparse
import datetime
import json
import os
from pathlib import Path
import platform
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parent
COUNTS = {"lifetime": 10, "ownership": 13, "polymorphism": 12, "templates": 10,
          "containers": 16, "interfaces": 7, "transaction": 8, "allocation": 7, "pimpl": 6}
ANNOTATED = ("lifetime", "ownership", "transaction", "polymorphism", "templates", "interfaces",
             "containers", "allocation", "pimpl")
LESSON_GROUPS = {"01-02": ("lifetime", "ownership", "transaction"),
                "03-04": ("polymorphism", "templates", "interfaces"),
                "05-06": ("containers", "allocation", "pimpl")}


def sources(name, annotated=False):
    filenames = ["pimpl.cpp", "pimpl_main.cpp"] if name == "pimpl" else [name + ".cpp"]
    return [str(ROOT / ("ko" if annotated else "original/src") / filename) for filename in filenames]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("mode", choices=("positive-direct", "compare", "negative", "cxx11"))
    parser.add_argument("--compiler", default="g++")
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--build", type=Path)
    args = parser.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)
    compiler = shutil.which(args.compiler)
    if not compiler:
        parser.error("컴파일러를 찾지 못했습니다: " + args.compiler)
    report = {
        "mode": args.mode,
        "executed_at_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "system": platform.platform(),
        "status": "running",
        "commands": [],
        "results": [],
        "selected_lessons": "01-06",
        "current_batch": "05-06",
        "runtime_check_invocations_by_lesson_batch": {
            batch: sum(COUNTS[name] for name in names) for batch, names in LESSON_GROUPS.items()},
        "current_batch_runtime_check_invocations": 29,
        "counting_note": "Original nine programs count 89 calls. Lessons 01-02 count 31 (10+13+8), lessons 03-04 count 29 (12+10+7), and current lessons 05-06 count 29 (16+7+6), totaling 89 across annotated programs. containers has 14 Expect call sites but executes 16 calls because one irreflexivity site runs for three items. ownership includes Expect(true) after a compile-time construction example. templates has three static_assert sites, interfaces one, ownership two, and pimpl_main one; these are separate from runtime calls. Annotation/compiler reruns are not independent conditions.",
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

    def normal(label, name, result):
        require(result.returncode == 0, label + ": 정상 프로그램 실패")
        counts = re.findall(rb"^PASS (\w+): ([0-9]+) checks\r?$", result.stdout, re.MULTILINE)
        require(counts == [(name.encode(), str(COUNTS[name]).encode())], label + ": 실제 검사 횟수 불일치")
        report["results"].append({"name": label, "outcome": "normal-pass", "runtime_check_invocations": COUNTS[name]})

    try:
        if compiler:
            version = run("compiler-version", [compiler, "--version"])
            require(version.returncode == 0, "컴파일러 버전 조회 실패")
            report["compiler"] = version.stdout.decode("utf-8", errors="replace").splitlines()[0]
        with tempfile.TemporaryDirectory(prefix="atlas-cpp-advanced-") as temporary:
            temp = Path(temporary)
            if args.mode in ("positive-direct", "compare"):
                # PowerOfTwo의 원본 제한 31은 타입의 비트 폭을 계산하지 않는다.
                # 환경 폭을 별도로 기록하며 원본 수정이나 새로운 논리 검사로 집계하지 않는다.
                probe_source = args.out / "unsigned-width-probe.cpp"
                probe_source.write_text(
                    '#include <iostream>\n#include <limits>\n'
                    "int main() { std::cout << std::numeric_limits<unsigned>::digits << '\\n'; }\n",
                    encoding="utf-8")
                probe_binary = temp / "unsigned-width-probe"
                probe_build = run("unsigned-width-build", [compiler, "-std=c++20", "-pedantic-errors",
                                  str(probe_source.resolve()), "-o", str(probe_binary)])
                require(probe_build.returncode == 0, "unsigned 환경 폭 조회 빌드 실패")
                probe_run = run("unsigned-width-run", [str(probe_binary)])
                require(probe_run.returncode == 0 and probe_run.stdout.strip().isdigit(),
                        "unsigned 환경 폭 조회 실행 실패")
                report["unsigned_value_bits"] = int(probe_run.stdout.strip())
                report["unsigned_width_probe_is_ctest_case"] = False
            if args.mode == "positive-direct":
                outputs = {}
                tasks = [(name, name, False) for name in COUNTS]
                tasks += [(name + "_ko", name, True) for name in ANNOTATED]
                for label, name, annotated in tasks:
                    binary = temp / label
                    include = ROOT / ("ko" if annotated else "original/src")
                    command = [compiler, "-std=c++20", "-g", "-O0", "-Wall", "-Wextra", "-Wpedantic",
                               "-Wconversion", "-Wshadow", "-Werror", "-I", str(include)]
                    built = run(label + "-build", command + sources(name, annotated) + ["-o", str(binary)])
                    require(built.returncode == 0, label + ": C++20 빌드 실패")
                    result = run(label + "-run", [str(binary)])
                    normal(label, name, result)
                    outputs[label] = result.stdout
                for name in ANNOTATED:
                    require(outputs[name] == outputs[name + "_ko"], name + ": 원본·주석판 stdout 불일치")
                report.update({"original_runtime_check_invocations": 89, "selected_lessons_runtime_check_invocations": 89,
                               "annotation_check_invocations_repeat_original": 89, "all_stdout_pairs_equal": True,
                               "stdout_pair_count": len(ANNOTATED), "stdout_paired_programs": list(ANNOTATED),
                               "stdout_normalization_applied": False, "cmake_ctest_executed": False})
            elif args.mode == "compare":
                require(args.build is not None, "--build 경로가 필요합니다")
                build = args.build.resolve()
                for name in ANNOTATED:
                    original = run(name + "-original", [str(build / "original" / name)])
                    annotated = run(name + "-korean", [str(build / (name + "_ko"))])
                    normal(name, name, original)
                    normal(name + "_ko", name, annotated)
                    require(original.stdout == annotated.stdout, name + ": 원본·주석판 stdout 불일치")
                report.update({"all_stdout_pairs_equal": True, "stdout_normalization_applied": False,
                               "stdout_pair_count": len(ANNOTATED), "stdout_paired_programs": list(ANNOTATED),
                               "selected_lessons_runtime_check_invocations": 89,
                               "annotation_check_invocations_repeat_original": 89})
            elif args.mode == "negative":
                for source in sorted((ROOT / "original/negative").glob("*.cpp")):
                    result = run(source.stem, [compiler, "-std=c++20", "-pedantic-errors", "-c", str(source),
                                               "-o", str(temp / (source.stem + ".o"))])
                    require(result.returncode == 1 and b"error:" in result.stdout,
                            source.name + ": 예상한 컴파일 거부가 아닙니다")
                    report["results"].append({"name": source.name, "outcome": "expected-compile-rejection",
                                               "compiler_exit_code": result.returncode})
                require(len(report["results"]) == 6, "컴파일 거부 원본은 여섯 개여야 합니다")
            else:
                for name in COUNTS:
                    result = run(name, [compiler, "-std=c++11", "-pedantic-errors", "-fsyntax-only",
                                        "-I", str(ROOT / "original/src")] + sources(name))
                    require(result.returncode == 1 and b"error:" in result.stdout,
                            name + ": C++11 예상 미지원 진단이 아닙니다")
                    report["results"].append({"name": name, "outcome": "cxx11-unsupported-expected-rejection",
                                               "compiler_exit_code": result.returncode})
                report["cxx11_supported"] = False
                report["cxx11_programs_executed"] = False
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
