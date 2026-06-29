Day56 ret2csu hard

Goal:
  Get code execution through the ret2csu-style gadgets and reach the success path.

Files:
  day56_csu_hard
  libcsu_hard.so
  flag.txt

Run:
  ./day56_csu_hard

Suggested checks:
  file ./day56_csu_hard
  checksec ./day56_csu_hard
  objdump -d -M intel ./day56_csu_hard
  readelf -rW ./day56_csu_hard
  readelf -S ./day56_csu_hard

Constraints:
  - Source is intentionally not included.
  - The main binary is stripped.
  - Do not patch the binary.
  - Solve with ROP/ret2csu.

Target idea:
  You need to control a 3-argument call using the available gadgets.
  A writable function-pointer slot exists, but it does not initially point to the target function.
