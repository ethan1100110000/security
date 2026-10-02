#!/usr/bin/env bash
set -u

if [[ $# -ne 1 ]]; then
    printf 'usage: %s INPUT_FILE\n' "$0" >&2
    exit 2
fi

input=$1
if [[ ! -f "$input" ]]; then
    printf 'input file not found: %s\n' "$input" >&2
    exit 2
fi

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
repo_root=$(cd -- "$script_dir/../.." && pwd)

afl_bin="$repo_root/Day101-160/Day126/day126_cli_afl"
debug_bin="$repo_root/Day101-160/Day126/day126_cli_debug"

if [[ ! -x "$afl_bin" || ! -x "$debug_bin" ]]; then
    printf 'target binary missing or not executable\n' >&2
    exit 2
fi

input=$(realpath -- "$input")
input_sha=$(sha256sum -- "$input" | cut -d ' ' -f 1)
out_dir="$script_dir/replay_output"
mkdir -p -- "$out_dir"
prefix="$out_dir/$input_sha"

{
    printf 'input=%s\n' "$input"
    printf 'input_size=%s\n' "$(wc -c < "$input")"
    sha256sum -- "$input" "$afl_bin" "$debug_bin"
} > "$prefix.meta.txt"

for kind in afl debug; do
    if [[ "$kind" == afl ]]; then
        binary=$afl_bin
    else
        binary=$debug_bin
    fi

    "$binary" "$input" > "$prefix.$kind.out" 2> "$prefix.$kind.err"
    result=$?
    printf '%s exit=%d\n' "$kind" "$result" | tee -a "$prefix.meta.txt"
done
