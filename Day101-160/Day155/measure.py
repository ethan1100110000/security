#!/usr/bin/env python3
"""Day155 user-run local address samples: PIE 2x ASLR on/off x N runs.
ASLR-OFF via setarch -R can be denied by WSL/container security settings.
Never fabricate ASLR-OFF measurements if permission is denied.
"""
import argparse
import csv
import platform
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
BINS = {"no_pie": ROOT / "addr_no_pie", "pie": ROOT / "addr_pie"}
PAT = re.compile(r"(main|helper|stack|libc_puts|main_minus_helper)=0x([0-9a-fA-F]+)")

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--aslr", choices=["on", "off"], required=True)
    parser.add_argument("--runs", type=int, default=10)
    args = parser.parse_args()
    if args.runs < 1:
        parser.error("--runs must be >= 1")
    if args.aslr == "off" and not shutil.which("setarch"):
        sys.exit("ASLR-OFF blocked: setarch is not installed; do not invent results.")
    outrows = []
    for flavor, binary in BINS.items():
        if not binary.is_file():
            sys.exit("Compile first with: bash build.sh")
        for i in range(1, args.runs + 1):
            cmd = [str(binary)]
            if args.aslr == "off":
                cmd = ["setarch", platform.machine(), "-R"] + cmd
            proc = subprocess.run(cmd, capture_output=True, text=True, cwd=ROOT)
            matches = dict(PAT.findall(proc.stdout))
            if proc.returncode != 0 or len(matches) != 5:
                print(f"RUN FAILED [{flavor} {args.aslr} #{i}].", file=sys.stderr)
                print("command: " + " ".join(cmd), file=sys.stderr)
                print((proc.stdout + proc.stderr).strip(), file=sys.stderr)
                if args.aslr == "off":
                    print("WSL/OS may deny ADDR_NO_RANDOMIZE (Operation not permitted). "
                          "Record the limitation; do not change system-wide ASLR.", file=sys.stderr)
                sys.exit(2)
            outrows.append({"binary": flavor, "aslr": args.aslr, "trial": i, **{
                key: f"0x{int(value, 16):x}" for key, value in matches.items()
            }})
    output = ROOT / f"measure_aslr-{args.aslr}.csv"
    fields = ["binary", "aslr", "trial", "main", "helper", "stack",
              "libc_puts", "main_minus_helper"]
    with output.open("w", newline="") as fh:
        writer = csv.DictWriter(fh, fieldnames=fields)
        writer.writeheader()
        writer.writerows(outrows)
    print("saved:", output.name)
    for flavor in BINS:
        rows = [r for r in outrows if r["binary"] == flavor]
        counts = {k: len({r[k] for r in rows})
                  for k in ["main", "helper", "stack", "libc_puts", "main_minus_helper"]}
        print(flavor, f"trials={len(rows)}", "unique-values:", counts)

if __name__ == "__main__":
    main()
