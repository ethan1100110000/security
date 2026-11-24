#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
base64 -d day158_probe.elf.xz.b64 | xz -d > day158_probe.elf
chmod +x day158_probe.elf
echo "009e49ee78b794c9345ca51912823b86ebd97b387c951da2cfac171355ed7146  day158_probe.elf" | sha256sum -c -
echo "Ready: ./day158_probe.elf"
