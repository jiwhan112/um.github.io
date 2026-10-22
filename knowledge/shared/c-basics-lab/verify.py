#!/usr/bin/env python3
"""Run from a repository checkout: python3 knowledge/shared/c-basics-lab/verify.py.
Writes verification.json only after all expected outcomes have been checked.
"""
from pathlib import Path
import hashlib, json, os, platform, subprocess, tempfile
from datetime import datetime, timezone
ROOT = Path(__file__).resolve().parents[3]
HERE = Path(__file__).resolve().parent
CC = os.environ.get('CC', 'gcc')
records = []
with tempfile.TemporaryDirectory(prefix='c-basics-') as temp:
    tmp = Path(temp)
    def run(args, success=True, expected=None):
        proc = subprocess.run(args, cwd=ROOT, capture_output=True, text=True)
        norm = lambda s: s.replace(str(tmp), '<build>').replace(str(ROOT) + '/', '')
        record = dict(command=[norm(str(a)) for a in args], exit_code=proc.returncode,
                      stdout=norm(proc.stdout), stderr=norm(proc.stderr), expected_success=success)
        records.append(record)
        assert (proc.returncode == 0) == success, record
        if expected is not None:
            assert proc.stdout == expected, record
        return proc.stdout
    version = run([CC, '--version']).splitlines()[0]
    flags = ['-std=c11', '-Wall', '-Wextra', '-Wpedantic', '-Werror']
    variants = {'strict': [], 'ubsan': ['-fsanitize=undefined', '-fno-sanitize-recover=undefined', '-g']}
    for mode, extra in variants.items():
        for n in range(1, 4):
            source = ROOT / f'knowledge/c-basics-{n:02}/main.c'
            exe = tmp / f'{mode}-{n}'
            run([CC, *flags, *extra, str(source), '-o', str(exe)])
            output = run([str(exe)])
            if n == 1: assert output == 'Hello, C!\n'
            if n == 2: assert output.startswith('score=85 total=0 average=2.5\n')
            if n == 3: assert output == 'division=2 late=2.0 early=2.5\nflags=4 overflow_rejected=1\n'
        # Exercise the real lesson's guard by changing only its inputs and expected postcondition.
        original = (ROOT / 'knowledge/c-basics-03/main.c').read_text()
        cases = [('INT_MAX','1',1,'INT_MAX'),('INT_MAX','0',0,'INT_MAX'),
                 ('INT_MAX - 1','1',0,'INT_MAX'),('INT_MIN','-1',1,'INT_MIN'),
                 ('INT_MIN + 1','-1',0,'INT_MIN'),('INT_MIN','0',0,'INT_MIN'),
                 ('-1','1',0,'0'),('0','INT_MIN',0,'INT_MIN'),('0','INT_MAX',0,'INT_MAX')]
        for i, (left, right, rejected, result) in enumerate(cases):
            changed = original.replace('int left = INT_MAX;', f'int left = {left};').replace('int right = 1;', f'int right = {right};')
            changed = changed.replace('assert(rejected == 1 && left == INT_MAX);', f'assert(rejected == {rejected} && left == {result});')
            source = tmp / f'boundary-{i}.c'; source.write_text(changed)
            exe = tmp / f'boundary-{mode}-{i}'
            run([CC, *flags, *extra, str(source), '-o', str(exe)])
            run([str(exe)], expected=f'division=2 late=2.0 early=2.5\nflags=4 overflow_rejected={rejected}\n')
    hello = ROOT / 'knowledge/c-basics-01/main.c'
    for option, suffix in [('-E','i'),('-S','s'),('-c','o')]:
        output = tmp / ('hello.' + suffix)
        run([CC, '-std=c11', option, str(hello), '-o', str(output)])
        assert output.stat().st_size > 0
    run([CC, str(tmp / 'hello.o'), '-o', str(tmp / 'hello')])
    run([str(tmp / 'hello')], expected='Hello, C!\n')
    run([CC, '-std=c11', '-c', str(HERE / 'compile-error.c'), '-o', str(tmp/'bad.o')], success=False)
    run([CC, '-std=c11', '-c', str(HERE / 'link-error.c'), '-o', str(tmp/'missing.o')])
    run([CC, str(tmp/'missing.o'), '-o', str(tmp/'missing')], success=False)
files = [ROOT/f'knowledge/c-basics-{n:02}/main.c' for n in range(1,4)] + [HERE/'compile-error.c', HERE/'link-error.c', HERE/'verify.py']
report = dict(status='pass', tested_at_utc=datetime.now(timezone.utc).isoformat(),
              platform=platform.platform(), compiler=version, language='C11',
              configurations=list(variants), distinct_addition_boundary_cases=len(cases),
              normal_programs=3, source_files=[dict(path=str(p.relative_to(ROOT)), sha256=hashlib.sha256(p.read_bytes()).hexdigest()) for p in files],
              limitation='GCC on this host only; not MSVC or other architectures. UBSan checks executed paths, not a proof for all inputs.',
              commands=records)
(HERE/'verification.json').write_text(json.dumps(report, ensure_ascii=False, indent=2)+'\n')
print(f'PASS: {len(records)} commands; 3 programs and {len(cases)} boundary cases in 2 configurations; 2 expected build failures.')
