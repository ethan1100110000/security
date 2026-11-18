#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
gcc -O0 -g -Wall -Wextra -fno-pie -no-pie addr_probe.c -ldl -o addr_no_pie
gcc -O0 -g -Wall -Wextra -fPIE -pie addr_probe.c -ldl -o addr_pie
printf 'Built: addr_no_pie, addr_pie\n'
printf 'ELF type (ET_EXEC vs ET_DYN):\n'
readelf -hW addr_no_pie | grep 'Type:'
readelf -hW addr_pie    | grep 'Type:'
