#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
base64 -d day157_relay.bin.xz.b64 | xz -d > day157_relay.elf
chmod +x day157_relay.elf
echo "1bf0156dbee17fdcf873907728c86c560dc43f5ef89b7a1d6bfe6b7163329bca  day157_relay.elf" | sha256sum -c -
echo "Ready: ./day157_relay.elf"
