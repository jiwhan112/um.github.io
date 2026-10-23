#!/usr/bin/env python3
"""Verify inserted-comment provenance without lexing/reformatting C++ strings."""
import hashlib
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
KNOWLEDGE = HERE.parent.parent
MARKER = b'// [\xed\x95\x9c\xea\xb5\xad\xec\x96\xb4 \xed\x95\xb4\xec\x84\xa4] '

def sha(data):
    return hashlib.sha256(data).hexdigest()

def main():
    manifest = json.loads((HERE / 'annotation-manifest.json').read_text())
    comments = 0
    for entry in manifest['files']:
        original = (KNOWLEDGE / entry['original']).read_bytes()
        annotated = (KNOWLEDGE / entry['annotated']).read_bytes()
        assert sha(original) == entry['original_sha256'], entry['original']
        assert sha(annotated) == entry['annotated_sha256'], entry['annotated']
        inserted = set(entry['inserted_comment_lines'])
        assert len(inserted) == len(entry['inserted_comment_lines'])
        lines = annotated.splitlines(keepends=True)
        assert all(1 <= n <= len(lines) for n in inserted)
        restored = []
        for n, line in enumerate(lines, 1):
            if n in inserted:
                assert line.startswith(MARKER) and line.endswith(b'\n')
                assert not line.rstrip().endswith(b'\\')
                assert n == 1 or not lines[n-2].rstrip().endswith(b'\\')
                comments += 1
            else:
                assert not line.startswith(MARKER)
                restored.append(line)
        assert b''.join(restored) == original, entry['annotated']
    print(f"PASS {len(manifest['files'])} complete files; {comments} standalone Korean comments; original bytes reconstructed exactly")
    print('No general C++ lexer claim: insertion locations were reviewed outside literals/comments/directive continuations.')

if __name__ == '__main__':
    main()
