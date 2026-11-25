#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
base64 -d day159_stack.elf.xz.b64 | xz -d > day159_stack.elf
chmod +x day159_stack.elf
echo "3512168bf96fd11b8337dc37e22b20257374eabeda5ab95ba612d0578a81452e  day159_stack.elf" | sha256sum -c -
echo "Ready: ./day159_stack.elf"
