#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
printf '%s\n' "All commands and output in this shell will be appended to day159_session.log."
printf '%s\n' "Exit this shell normally at the end of study. Do not close the outer terminal."
script -q -f -a day159_session.log -c "bash --noprofile --norc"
