# Day51 FSB Write 1: Byte Overwrite

Goal: Use one format-string input to overwrite a global 1-byte variable to `0x7f`.

Rules:
- Try binary-first.
- Do not read `source.c` until after solving or getting stuck.

Expected checks:
- Find FSB argument offset.
- Use `%hhn` or `fmtstr_payload(..., write_size="byte")`.
- Verify auth before/after.
