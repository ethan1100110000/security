#!/bin/sh
set -eu
cd "$(dirname "$0")"
gcc -O0 -fstack-protector-all -fPIE -pie -Wl,-z,relro,-z,now -Wl,-z,noexecstack -s challenge.c -o archive_desk
printf '%s\n' 'Built ./archive_desk'
