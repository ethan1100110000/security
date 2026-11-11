#!/bin/sh
# Reference reproduction script, prepared by assistant; user has not run it.
# Compiles to uniquely named binaries so it will not overwrite local files.
set -eu
cd "$(dirname "$0")"
gcc -O0 -fstack-protector-all -fPIE -pie -Wl,-z,relro,-z,now -Wl,-z,noexecstack -s experiment_submit64.c -o archive_desk_repro
gcc -std=c11 -Wall -Wextra packet_layout_reference.c -o packet_layout_repro
printf '%s\n' 'Reference executables built: archive_desk_repro, packet_layout_repro'
