# Day52 FSB leak + write composite

Solve from the binary first. Source is intentionally not included in the zip.

Goal: use FSB leak and FSB write in one run.

Expected route:
- find FSB offset
- leak a PIE/code pointer
- compute PIE base
- overwrite a writable GOT entry with hidden success function
- trigger the overwritten function

