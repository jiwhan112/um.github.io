#!/usr/bin/env python3
"""Configure, build and run this CPU lab once, saving logs in the chosen build dir."""
from __future__ import annotations
import argparse
from pathlib import Path
import shutil
import subprocess
import sys


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', help='C++ compiler name or executable path')
    parser.add_argument('--build-dir', type=Path, default=Path('build-verify'))
    parser.add_argument('--config', default='Debug')
    parser.add_argument('--sanitizers', action='store_true')
    args = parser.parse_args()
    root = Path(__file__).resolve().parent
    build = args.build_dir.resolve()
    if build == root or build in root.parents:
        parser.error('Use a dedicated build directory, not the source or its parent.')
    cmake = shutil.which('cmake')
    ctest = shutil.which('ctest')
    if not cmake or not ctest:
        parser.error('CMake and CTest must be installed and available on PATH.')
    configure = [cmake, '-S', str(root), '-B', str(build),
                 f'-DCMAKE_BUILD_TYPE={args.config}']
    if args.compiler:
        compiler = shutil.which(args.compiler)
        if not compiler:
            parser.error(f'Compiler not found: {args.compiler}')
        configure.append(f'-DCMAKE_CXX_COMPILER={compiler}')
    configure.append(f'-DENABLE_SANITIZERS={"ON" if args.sanitizers else "OFF"}')
    commands = [
        ('configure', configure),
        ('build', [cmake, '--build', str(build), '--config', args.config, '--parallel', '2']),
        ('test', [ctest, '--test-dir', str(build), '-C', args.config, '-V', '--output-on-failure']),
    ]
    build.mkdir(parents=True, exist_ok=True)
    for name, command in commands:
        print(f'[{name}] ' + ' '.join(command), flush=True)
        result = subprocess.run(command, stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT, text=True,
                                encoding='utf-8', errors='replace', timeout=300)
        (build / f'verify-{name}.log').write_text(result.stdout, encoding='utf-8')
        print(result.stdout)
        if result.returncode:
            return result.returncode
    return 0


if __name__ == '__main__':
    try:
        raise SystemExit(main())
    except (OSError, subprocess.TimeoutExpired) as exc:
        print(f'Verification failed: {exc}', file=sys.stderr)
        raise SystemExit(1)

