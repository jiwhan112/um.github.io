#!/usr/bin/env python3
"""Compile and execute the six C11 lessons; record evidence, not universal proof."""
import datetime
import hashlib
import json
import pathlib
import platform
import subprocess
import tempfile

HERE = pathlib.Path(__file__).resolve().parent
ROOT = HERE.parents[2]
CONFIGS = {
    "strict": ["-std=c11", "-O0", "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-pedantic-errors", "-fno-common"],
    "ubsan": ["-std=c11", "-O1", "-g", "-fsanitize=undefined,bounds", "-fno-sanitize-recover=all", "-Wall", "-Wextra", "-Wpedantic", "-Werror"],
}

def run(command, cwd):
    result = subprocess.run(command, cwd=cwd, text=True, capture_output=True, timeout=60)
    if result.returncode:
        raise RuntimeError(f"{command}: exit {result.returncode}\n{result.stdout}\n{result.stderr}")
    return result

def main():
    previous_path = HERE / "verification.json"
    previous = json.loads(previous_path.read_text()) if previous_path.exists() else {}
    previous_programs = {p["id"]: p for p in previous.get("programs", [])}
    executions = 0
    evidence = {
        "schema_version": 1,
        "status": "pass_local_cpu_only",
        "tested_at_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "host": platform.platform(),
        "compiler": run(["gcc", "--version"], ROOT).stdout.splitlines()[0],
        "language": "C11",
        "configurations": CONFIGS,
        "programs": [],
        "limitations": [
            "Only the supplied test paths and rejection cases were executed; this is not exhaustive proof.",
            "Assertions enabled; NDEBUG not defined. UBSan and bounds checks do not establish absence of all memory defects.",
            "Windows/MSVC and other C implementations were not tested.",
            "Browser rendering and public publication require separate CI and live evidence.",
        ],
    }
    with tempfile.TemporaryDirectory(prefix="atlas-c09-14-") as temp:
        for number in range(9, 15):
            directory = ROOT / "knowledge" / f"c-basics-{number:02}"
            inputs = [directory / "main.c"]
            if number == 13:
                inputs.append(directory / "inventory.c")
            sources = inputs + sorted(directory.glob("*.h"))
            entry = {"id": directory.name, "sources": [], "runs": {}}
            for source in sources:
                data = source.read_bytes()
                entry["sources"].append({"path": str(source.relative_to(ROOT)), "bytes": len(data), "sha256": hashlib.sha256(data).hexdigest()})
            old = previous_programs.get(directory.name)
            if (old and old["sources"] == entry["sources"] and
                    previous.get("configurations") == CONFIGS and
                    previous.get("compiler") == evidence["compiler"] and
                    all(r["exit_code"] == 0 and r["compile_exit_code"] == 0 for r in old["runs"].values())):
                old.setdefault("tested_at_utc", previous["tested_at_utc"])
                old["unchanged_source_evidence_reused"] = True
                evidence["programs"].append(old)
                continue
            entry["tested_at_utc"] = evidence["tested_at_utc"]
            entry["unchanged_source_evidence_reused"] = False
            for name, flags in CONFIGS.items():
                output = pathlib.Path(temp) / f"{directory.name}-{name}"
                command = ["gcc", *flags, *[str(p.relative_to(ROOT)) for p in inputs], "-o", str(output)]
                compilation = run(command, ROOT)
                execution = run([str(output)], temp)
                executions += 1
                entry["runs"][name] = {"command": [*command[:-1], "<temporary-executable>"], "compile_exit_code": compilation.returncode, "compile_stderr": compilation.stderr, "exit_code": execution.returncode, "stdout": execution.stdout, "stderr": execution.stderr}
            if entry["runs"]["strict"]["stdout"] != entry["runs"]["ubsan"]["stdout"]:
                raise RuntimeError(f"Output mismatch: {directory.name}")
            evidence["programs"].append(entry)
    (HERE / "verification.json").write_text(json.dumps(evidence, ensure_ascii=False, indent=2) + "\n")
    print(f"PASS: 6 C11 programs; {executions} fresh strict/UBSan executions; unchanged source evidence reused for {(12-executions)//2} programs")

if __name__ == "__main__":
    main()
