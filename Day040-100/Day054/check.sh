#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"

file ./day54_pivot

echo
if command -v checksec >/dev/null 2>&1; then
    checksec ./day54_pivot
else
    echo "checksec not found; using readelf fallback"
    readelf -h ./day54_pivot | grep Type
    readelf -W -l ./day54_pivot | grep GNU_STACK
fi

echo
echo "[*] symbols"
nm -n ./day54_pivot | grep -E ' fake_stack| main| vuln| gadget_'

echo
echo "[*] relocations"
readelf -rW ./day54_pivot | grep -E 'puts|read|setvbuf' || true

echo
echo "[*] gadgets"
objdump -d -M intel ./day54_pivot | grep -A2 -E '<gadget_pop_rdi>|<gadget_ret>|<gadget_leave_ret>'
