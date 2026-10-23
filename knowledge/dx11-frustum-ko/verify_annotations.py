#!/usr/bin/env python3
"""Check comment-only provenance without parsing or removing C++ string contents."""
from pathlib import Path
import json
import hashlib
HERE=Path(__file__).resolve().parent
ROOT=HERE.parent

def main():
    entries=json.loads((HERE/'annotation-manifest.json').read_text())['files']
    count=0
    for e in entries:
        original=(ROOT/e['original']).read_bytes()
        annotated=(ROOT/e['annotated']).read_bytes()
        assert hashlib.sha256(original).hexdigest()==e['original_sha256'],e['original']
        assert hashlib.sha256(annotated).hexdigest()==e['annotated_sha256'],e['annotated']
        lines=annotated.splitlines(keepends=True)
        assert len(lines)==len(original.splitlines(keepends=True)), 'physical line count changed'
        seen=set()
        for segment in e.get('appended_comment_segments',[]):
            n=segment['line']; suffix=segment['suffix'].encode()
            assert n not in seen and 1<=n<=len(lines)
            seen.add(n)
            assert suffix.startswith('// [한국어 해설]'.encode()) or suffix.startswith(' // [한국어 해설]'.encode())
            ending=b'\r\n' if lines[n-1].endswith(b'\r\n') else b'\n' if lines[n-1].endswith(b'\n') else b''
            content=lines[n-1][:-len(ending)] if ending else lines[n-1]
            assert content.endswith(suffix)
            lines[n-1]=content[:-len(suffix)]+ending
            assert not lines[n-1].rstrip().endswith(b'\\'), 'continuation line insertion'
            count+=1
        assert not e.get('inserted_comment_lines'), 'this package uses suffixes only'
        assert b''.join(lines)==original,e['annotated']
    print(f'PASS {len(entries)} complete files / {count} Korean comment suffixes; exact original bytes and line counts restored')
    print('Insertion locations reviewed outside literals/block comments; not a general C++ tokenizer.')
if __name__=='__main__': main()
