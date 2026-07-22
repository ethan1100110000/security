#!/usr/bin/env bash
set -euo pipefail

make clean
make

echo
echo "[+] build complete"
echo "[+] target: ./day79"
echo "[+] run checksec with: make check"
