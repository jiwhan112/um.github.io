#!/usr/bin/env python3
"""Verify preserved DX11 13–18 files without normalizing any source bytes."""

import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import re
import sys


class IntegrityError(ValueError):
    pass


def require(condition, message):
    if not condition:
        raise IntegrityError(message)


def repository_file(root, value):
    require(isinstance(value, str) and value, "file path must be a nonempty string")
    relative = PurePosixPath(value)
    require(not relative.is_absolute() and ".." not in relative.parts and "\\" not in value,
            f"unsafe repository path: {value}")
    path = (root / relative).resolve()
    require(path.is_relative_to(root), f"path escapes repository: {value}")
    require(path.is_file(), f"missing file: {value}")
    return path


def fenced_payloads(data, source_path):
    """Return all Markdown fence payloads in order, retaining LF/CRLF before close."""
    lines = data.splitlines(keepends=True)
    opening = re.compile(rb"(?P<fence>`{3,}|~{3,})[^\r\n]*(?:\r?\n|\r|$)")
    blocks = []
    line_number = 0
    while line_number < len(lines):
        match = opening.fullmatch(lines[line_number])
        if not match:
            line_number += 1
            continue
        fence = match.group("fence")
        closing = re.compile(re.escape(fence[:1]) + b"{" + str(len(fence)).encode("ascii")
                             + rb",}[ \t]*(?:\r?\n|\r|$)")
        start = line_number + 1
        line_number = start
        while line_number < len(lines) and not closing.fullmatch(lines[line_number]):
            line_number += 1
        require(line_number < len(lines), f"unclosed fence in {source_path} at line {start}")
        # Joining byte lines keeps the payload's terminal newline; no strip/rstrip.
        blocks.append(b"".join(lines[start:line_number]))
        line_number += 1
    return blocks


def fnv1a64(data):
    value = 0xCBF29CE484222325
    for byte in data:
        value = ((value ^ byte) * 0x100000001B3) & 0xFFFFFFFFFFFFFFFF
    return f"{value:016x}"


def verify(root, manifest_path):
    manifest = json.loads(manifest_path.read_bytes())
    require(manifest.get("schemaVersion") == 1, "unsupported source-integrity schemaVersion")
    require(re.fullmatch(r"[0-9a-f]{40}", manifest.get("baselineCommit", "")) is not None,
            "baselineCommit must identify the audited Git commit")
    entries = manifest.get("files")
    pages = manifest.get("pages")
    require(isinstance(entries, list) and entries, "manifest must contain files")
    require(isinstance(pages, list) and pages, "manifest must contain page evidence")
    data_by_path = {}
    for entry in entries:
        require(isinstance(entry, dict), "file entry must be an object")
        name = entry.get("path")
        path = repository_file(root, name)
        require(name not in data_by_path, f"duplicate file entry: {name}")
        require(isinstance(entry.get("kind"), str) and entry["kind"], f"missing kind: {name}")
        require(type(entry.get("bytes")) is int and entry["bytes"] >= 0, f"invalid bytes: {name}")
        require(re.fullmatch(r"[0-9a-f]{64}", entry.get("sha256", "")) is not None,
                f"invalid SHA-256: {name}")
        data = path.read_bytes()
        require(len(data) == entry["bytes"], f"byte count mismatch: {name}: {len(data)} != {entry['bytes']}")
        require(hashlib.sha256(data).hexdigest() == entry["sha256"], f"SHA-256 mismatch: {name}")
        if "fnv64" in entry:
            require(fnv1a64(data) == entry["fnv64"], f"FNV-1a mismatch: {name}")
        data_by_path[name] = data

    blocks_by_path = {}
    reconstructed = 0
    log_count = 0
    for entry in entries:
        name = entry["path"]
        source_name = entry.get("sourcePath")
        indexes = entry.get("codeBlockIndexesZeroBased")
        require((source_name is None) == (indexes is None),
                f"sourcePath and codeBlockIndexesZeroBased must be paired: {name}")
        if source_name is None:
            continue
        require(source_name in data_by_path, f"source snapshot is not hash-pinned: {source_name}")
        require(isinstance(indexes, list) and indexes and all(type(i) is int and i >= 0 for i in indexes),
                f"invalid source block indexes: {name}")
        require(indexes == sorted(set(indexes)), f"source block indexes must be unique and ordered: {name}")
        if source_name not in blocks_by_path:
            blocks_by_path[source_name] = fenced_payloads(data_by_path[source_name], source_name)
        parts = blocks_by_path[source_name]
        require(indexes[-1] < len(parts), f"source block index outside {source_name}: {name}")
        expected = b"".join(parts[index] for index in indexes)
        require(data_by_path[name] == expected, f"Notion fenced payload byte mismatch: {name}")
        reconstructed += 1
        log_count += name.endswith(".log")

    page_paths = set()
    for page in pages:
        name = page.get("path")
        require(name in data_by_path, f"page snapshot is not hash-pinned: {name}")
        require(name not in page_paths, f"duplicate page snapshot: {name}")
        page_paths.add(name)
        prefix = page.get("preservedPrefixBytes", 0)
        require(type(prefix) is int and 0 <= prefix <= len(data_by_path[name]), f"invalid preserved prefix: {name}")
        body = data_by_path[name][prefix:]
        require(len(body) == page.get("bodyBytes"), f"page body byte count mismatch: {name}")
        require(hashlib.sha256(body).hexdigest() == page.get("bodySha256"), f"page body SHA-256 mismatch: {name}")

    print(f"PASS source-integrity: {len(entries)} files, {len(pages)} page bodies, "
          f"{reconstructed} exact fenced reconstructions ({log_count} historical log files)")
    print("Current Notion text equality does not establish historical artifact byte identity.")
    return 0


def main():
    default_root = Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=default_root)
    parser.add_argument("--manifest", type=Path,
                        default=Path("knowledge/verification/dx11-13-18/source-integrity.json"))
    args = parser.parse_args()
    root = args.root.resolve()
    manifest_path = args.manifest if args.manifest.is_absolute() else root / args.manifest
    try:
        return verify(root, manifest_path)
    except (IntegrityError, OSError, ValueError, TypeError, KeyError) as error:
        print(f"FAIL source-integrity: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
