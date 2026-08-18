# Day101 — Anti-debug 3: persistent branch patch

## 1. Original artifact

- Original SHA-256:
- ELF protections:
- Entry에서 찾은 main 후보와 근거:

## 2. Detection flow

- 탐지 함수 주소:
- 탐지 primitive/API:
- 원본 반환값:
- bool 변환:
- caller의 검사 명령:
- 탐지 시 도달하는 블록:

## 3. Dynamic bypass

- breakpoint:
- 변경한 레지스터/값:
- 현재 실행 결과:
- 재실행 결과:
- 지속되지 않는 이유:

## 4. Static branch patch plan

- 패치 대상 조건 분기:
- 원본 instruction bytes:
- 변경 instruction bytes:
- 대상 instruction의 virtual address:
- 포함된 LOAD segment의 p_vaddr:
- 포함된 LOAD segment의 p_offset:
- 계산한 file offset:

```text
file_offset = instruction_vaddr - segment_vaddr + segment_offset
```

- 이 분기를 선택한 이유:

## 5. Patch verification

- Patched SHA-256:
- objdump에서 확인한 변경:
- 재실행 후 anti-debug 결과:
- token 검증 결과:

## 6. Failure case

- 실패 입력 또는 잘못된 패치:
- 관찰 결과:
- 원인:

## 7. Bypass comparison

| 방식 | 개입 지점 | 원본 파일 변경 | 재실행 유지 | 부작용/주의점 |
|---|---|---:|---:|---|
| GDB API 반환값 변경 |  |  |  |  |
| GDB wrapper bool 변경 |  |  |  |  |
| 실행 중 분기 바이트 변경 |  |  |  |  |
| 바이너리 분기 정적 패치 |  |  |  |  |

## 8. CS summary

- 동적 우회와 정적 패치의 차이:
- virtual address와 file offset의 차이:
- 무결성 검사에 탐지될 수 있는 이유:
- 원본을 보존해야 하는 이유:
