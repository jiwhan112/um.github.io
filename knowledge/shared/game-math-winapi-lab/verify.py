#!/usr/bin/env python3
"""Rebuild portable tests in a fresh temporary directory; preserve per-run logs."""
from __future__ import annotations
import argparse
import datetime as dt
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', default='g++')
    parser.add_argument('--sanitizers', action='store_true')
    args = parser.parse_args()
    compiler = shutil.which(args.compiler)
    if not compiler or not shutil.which('cmake') or not shutil.which('ctest'):
        print('The selected compiler, cmake and ctest must be available on PATH.', file=sys.stderr)
        return 2
    root = Path(__file__).resolve().parent
    stamp = dt.datetime.now(dt.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    logs = root / 'validation_runs' / stamp
    logs.mkdir(parents=True, exist_ok=False)
    env = os.environ.copy()
    if args.sanitizers:
        env['ASAN_OPTIONS'] = 'detect_leaks=1'
        env['UBSAN_OPTIONS'] = 'halt_on_error=1'
    try:
        with tempfile.TemporaryDirectory(prefix='study-math-build-') as temporary:
            commands = [
                ('configure', ['cmake', '-S', str(root), '-B', temporary,
                               '-DCMAKE_BUILD_TYPE=Debug', f'-DCMAKE_CXX_COMPILER={compiler}',
                               f'-DENABLE_SANITIZERS={"ON" if args.sanitizers else "OFF"}']),
                ('build', ['cmake', '--build', temporary, '--config', 'Debug', '-j', '2']),
                ('ctest', ['ctest', '--test-dir', temporary, '-C', 'Debug', '-V']),
            ]
            for name, command in commands:
                result = subprocess.run(command, text=True, stdout=subprocess.PIPE,
                                        stderr=subprocess.STDOUT, env=env, timeout=180)
                (logs / f'{name}.log').write_text(result.stdout, encoding='utf-8')
                print(f'{name}: exit {result.returncode}')
                if result.returncode:
                    print(result.stdout, file=sys.stderr)
                    return result.returncode
    except (OSError, subprocess.TimeoutExpired) as error:
        print(str(error), file=sys.stderr)
        return 1
    print(f'Logs saved to {logs}')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
