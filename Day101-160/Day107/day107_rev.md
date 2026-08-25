# Day107 — Malware static 2: strings/imports

## Sample identity

- SHA-256:
- File type / architecture:
- PIE / NX / Canary / RELRO:
- Stripped:
- Execution performed: No

## Imported functions

| Import | Capability candidate | Caller/XREF | Reachable from main? |
|---|---|---|---|
| | | | |

## Interesting strings

| File offset | String | Initial hypothesis | XREF |
|---|---|---|---|
| | | | |

## Candidate 1

- Initial clue:
- Referencing function:
- Caller path:
- Argument flow:
- Return-value use:
- Confirmed observation:
- Reasonable inference:
- Unconfirmed claim:

## Candidate 2

- Initial clue:
- Referencing function:
- Caller path:
- Argument flow:
- Return-value use:
- Confirmed observation:
- Reasonable inference:
- Unconfirmed claim:

## String not visible in ordinary strings output

- Encoded bytes:
- Decode operation/key:
- Decoded value:
- Consumer of decoded buffer:
- Why ordinary strings missed it:

## Main-reachable flow

```text
main
└─
```

## Cross-verification

- Ghidra inference:
- readelf/objdump command:
- Relevant instruction or symbol evidence:
- Result:

## Capability assessment

### Confirmed

-

### Possible but unconfirmed

-

### Decoy or unreachable candidates

-

## Limitations

- Static analysis only; runtime behavior was not tested.
-

## Failure case

- A conclusion that strings/imports alone would get wrong:
- Evidence that corrects it:

