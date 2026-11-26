#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
mkdir -p bin
COMMON=(-O0 -g -Wall -Wextra -fno-builtin -fstack-protector-strong)
# Same source, baseline and three one-variable changes.
gcc "${COMMON[@]}" -fPIE -pie -Wl,-z,relro,-z,now,-z,noexecstack -o bin/lab_full_pie_nx lab160.c
gcc "${COMMON[@]}" -fPIE -pie -Wl,-z,relro,-z,lazy,-z,noexecstack -o bin/lab_partial_pie_nx lab160.c
gcc "${COMMON[@]}" -fno-pie -no-pie -Wl,-z,relro,-z,now,-z,noexecstack -o bin/lab_full_nopie_nx lab160.c
gcc "${COMMON[@]}" -fPIE -pie -Wl,-z,relro,-z,now,-z,execstack -o bin/lab_full_pie_execstack lab160.c
printf '%s\n' 'Built four variants in bin/. Now independently verify ELF protection settings.'
