#!/usr/bin/env sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)
build=$(mktemp -d)
trap 'rm -rf "$build"' EXIT HUP INT TERM
for lesson in c-basics-04 c-basics-05; do
    src="$root/knowledge/$lesson/main.c"
    gcc -std=c11 -Wall -Wextra -Wpedantic -Werror "$src" -o "$build/$lesson"
    "$build/$lesson"
    gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -fsanitize=undefined -fno-sanitize-recover=all "$src" -o "$build/$lesson-ubsan"
    "$build/$lesson-ubsan"
done

