#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
base=(-O0 -g -fno-omit-frame-pointer -fno-pie -no-pie -Wall -Wextra)
for canary in off on; do
  if [[ "$canary" == "on" ]]; then
    canary_flag=(-fstack-protector-all)
  else
    canary_flag=(-fno-stack-protector)
  fi
  for nx in off on; do
    if [[ "$nx" == "on" ]]; then
      nx_flag=(-Wl,-z,noexecstack)
    else
      nx_flag=(-Wl,-z,execstack)
    fi
    gcc "${base[@]}" "${canary_flag[@]}" "${nx_flag[@]}" condition_matrix.c -o "lab_canary-${canary}_nx-${nx}"
    printf 'built lab_canary-%s_nx-%s\n' "$canary" "$nx"
  done
done
