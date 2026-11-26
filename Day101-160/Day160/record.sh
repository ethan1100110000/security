#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
echo 'Append-only flushed transcript to day160_session.log.'
echo 'Exit normally; if interrupted, rerun this script to append.'
script -q -f -a day160_session.log -c "bash --noprofile --norc"
