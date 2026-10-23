#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT HUP INT TERM
for n in 06 07 08; do
  source="$root/knowledge/c-basics-$n/main.c"
  gcc -std=c11 -O0 -Wall -Wextra -Wpedantic -Werror -pedantic-errors -fno-common "$source" -o "$tmp/a$n"
  "$tmp/a$n"
  gcc -std=c11 -O1 -g -fsanitize=undefined,bounds -fno-sanitize-recover=all -Wall -Wextra -Wpedantic -Werror "$source" -o "$tmp/a$n-san"
  "$tmp/a$n-san"
  sha256sum "$source"
done
