#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
base64 -d day156_target.elf.xz.b64 | xz -dc > day156_target.elf
echo "616ff0a7e73176034294a9c3997c4d070a005a11d515417fbf59050e02a5f848  day156_target.elf" | sha256sum -c -
chmod +x day156_target.elf
echo "Ready: ./day156_target.elf"
