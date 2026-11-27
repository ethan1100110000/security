#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
printf '%s\n' 'Recording append-only session to day161_session.log.'
printf '%s\n' 'After finishing, exit the nested shell normally. If interrupted, use the same script to append.'
script -q -f -a day161_session.log -c "bash --noprofile --norc"
