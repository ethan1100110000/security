#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
target="day161_blind.elf"
expected="c6f57ee36ab304cb393b61c94e781e289fb7f4c91eaefbd6945f127e85034183"

if [[ ! -s "$target" ]]; then
    temp="$(mktemp "./.day161_binary.XXXXXXXX")"
    trap 'rm -f "$temp"' EXIT
    base64 -d day161_blind.elf.xz.b64 | xz -dc > "$temp"
    mv "$temp" "$target"
    trap - EXIT
fi
echo "$expected  $target" | sha256sum -c -
chmod +x "$target"
file "$target"
printf 'Day161 local binary ready. Source not included: blind assessment.\n'
