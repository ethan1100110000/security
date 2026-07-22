#!/usr/bin/env bash
set -euo pipefail

make clean
make

echo
echo "[+] target built: ./day79_menu"
echo "[+] protections:"
make check
