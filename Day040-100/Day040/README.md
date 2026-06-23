# Day40 Canary + PIE + BOF - ROP Emporium style lab

## Files

- `canary_pie_bof`: main challenge binary
- `libcanary_pie_bof.so`: shared library containing `pwnme()`
- `flag.txt`: success marker
- source files are included for review/rebuild

## Intended protections

Main binary:

- Canary: ON
- PIE: ON
- NX: ON

Shared library:

- Canary: ON
- PIE/shared object randomized
- NX: ON

## Run

```bash
./canary_pie_bof
```

If the shared library is not found:

```bash
LD_LIBRARY_PATH=. ./canary_pie_bof
```

## Suggested checks

```bash
checksec ./canary_pie_bof
checksec ./libcanary_pie_bof.so
nm -n ./canary_pie_bof | grep -E "main|win|useful"
objdump -d ./canary_pie_bof | grep -E "main|win|usefulGadgets|pwnme@plt"
objdump -d ./libcanary_pie_bof.so | grep -E "pwnme|printf|read|stack_chk"
```

## Goal

Stage 1:

```text
Use the format string bug to leak:
1. stack canary
2. PIE/code pointer
```

Stage 2:

```text
Overflow buf and preserve the leaked canary.

payload =
padding_to_canary
+ canary
+ dummy saved rbp
+ optional ret
+ PIE-adjusted win
```

## Notes

Canary candidates usually:

- look like an 8-byte value
- end with `00`

PIE candidates usually:

- look like a main-binary code pointer
- when subtracting a known code offset, produce a page-aligned base ending in `000`

## Success check

The program should print:

```text
ROPE{day40_canary_pie_bof_success}
```
