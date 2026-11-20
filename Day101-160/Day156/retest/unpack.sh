#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
base64 -d day156_retest.elf.xz.b64 | xz -dc > day156_retest.elf
echo "565d297112871b10645becfa1527b7a589af7392a99cfb2ea8c1793b461430cc  day156_retest.elf" | sha256sum -c -
chmod +x day156_retest.elf
