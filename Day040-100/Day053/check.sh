#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"

echo "[*] file"
file ./day53_pivot

echo
echo "[*] checksec"
if command -v checksec >/dev/null 2>&1; then
    checksec ./day53_pivot
else
    echo "checksec not found"
fi

echo
echo "[*] symbols"
nm -n ./day53_pivot | grep -E "win|fake_stack|vuln|main" || true

echo
echo "[*] leave; ret"
if command -v ROPgadget >/dev/null 2>&1; then
    ROPgadget --binary ./day53_pivot | grep "leave ; ret" || true
else
    objdump -d -M intel ./day53_pivot | grep -B1 -A1 "leave" || true
fi
