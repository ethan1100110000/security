#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
bash unpack.sh
src=fixed_submitted.c
printf '%s\n' '=== environment ==='
uname -srmo
gcc --version | head -n 1
getconf GNU_LIBC_VERSION
sha256sum day156_retest.elf "$src"
printf '%s\n' '=== build ===' 'gcc -O0 -g -fstack-protector-all -fPIE -pie -Wl,-z,relro,-z,now fixed_submitted.c -o <temporary>/fixed'
gcc -O0 -g -fstack-protector-all -fPIE -pie -Wl,-z,relro,-z,now "$src" -o "$tmp/fixed"
for n in 23 24 25; do
    printf '3:' > "$tmp/input"
    printf '%*s' "$n" '' | tr ' ' a >> "$tmp/input"
    printf '\n' >> "$tmp/input"
    printf '\n=== body length %s ===\n' "$n"
    cat "$tmp/input"
    for exe in ./day156_retest.elf "$tmp/fixed"; do
        printf '%s\n' "$exe"
        rc=0
        "$exe" < "$tmp/input" || rc=$?
        printf 'exit=%s\n' "$rc"
    done
done
