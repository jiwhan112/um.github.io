#!/usr/bin/env python3
"""Verify unchanged sources against preserved executed CI evidence; do not rerun CPU tests."""
import datetime
import hashlib
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
LAB = HERE.parent / "cpp-advanced-lab"
CI = LAB / "verification/github-39571778548"

def matches(path, record):
    data = path.read_bytes()
    assert len(data) == record["bytes"], str(path)
    assert hashlib.sha256(data).hexdigest() == record["sha256"], str(path)

def main():
    manifest = json.loads((CI / "artifact-manifest.json").read_text())
    for record in manifest["files"]:
        matches(CI / record["path"], record)
    sources = {}
    for config in ["gcc-cxx20", "clang-cxx20-asan-ubsan"]:
        text = (CI / config / "environment-and-integrity.txt").read_text()
        historical, _ = json.JSONDecoder().raw_decode(text[text.index("{"):])
        for path, record in historical["annotated_files"].items():
            matches(LAB / path, record)
            sources[path] = {"bytes": record["bytes"], "sha256": record["sha256"]}
        for path, record in historical["artifacts"].items():
            matches(LAB / path, record)
    evidence = {
        "schema_version": 1,
        "status": "pass_source_identity_prior_cpu_evidence_reused",
        "checked_at_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "prior_run_url": manifest["run_url"],
        "prior_source_commit": manifest["source_commit"],
        "prior_manifest": str(CI.relative_to(HERE.parents[2]) / "artifact-manifest.json"),
        "preserved_raw_files_verified": len(manifest["files"]),
        "annotated_source_files_verified": sources,
        "new_cpu_compilation_or_execution": False,
        "presentation_and_publication": "Separate gates; not established by this source-identity check.",
        "limitations": [
            "Reuses exact unchanged sources and preserved results, not new execution on a new compiler.",
            "Prior run: GCC C++20; Clang C++20 ASan/UBSan. C++11 was unsupported and not executed.",
            "89 runtime check calls are not 89 independent proven properties; repeated annotations/compilers are not additional conditions.",
            "No Windows/MSVC, ABI, concurrency, performance or exhaustive input claims.",
        ],
    }
    (HERE / "verification.json").write_text(json.dumps(evidence, ensure_ascii=False, indent=2) + "\n")
    print(f"PASS: {len(sources)} unchanged annotated files and {len(manifest['files'])} preserved raw evidence files")

if __name__ == "__main__":
    main()
