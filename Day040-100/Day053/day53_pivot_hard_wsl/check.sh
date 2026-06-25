#!/usr/bin/env bash
set -euo pipefail
file ./day53_pivot_hard
checksec --file=./day53_pivot_hard || true
echo
echo '[sections]'
readelf -S ./day53_pivot_hard | grep -E '^[[:space:]]*\[[ 0-9]+\][[:space:]]+\.(text|plt|got|data|bss)'
echo
echo '[segments]'
readelf -l ./day53_pivot_hard | grep -A3 LOAD
