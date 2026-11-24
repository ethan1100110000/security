#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
base64 -d day158_probe.elf.xz.b64 | xz -d > day158_probe.elf
chmod +x day158_probe.elf
echo "75ba0e98ba5bf01486c6726d4bd297d47af64eda70477c666526dffab6371735  day158_probe.elf" | sha256sum -c -
echo "Ready: ./day158_probe.elf"
