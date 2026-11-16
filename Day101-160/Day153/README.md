# Day153 / FINAL W06 1/4 — first-look stripped ELF (진행 중)

## 대상
- `target_elf.gz.b64`: **교사가 생성한 로컬 교육용 x86-64 Linux stripped ELF**를 gzip+base64로 보관한 것. 소스는 첫 독립 분석 전에는 공개하지 않음.
- 본 과제는 사용자 소유의 격리된 로컬 바이너리 학습용이며 타인 서비스/호스트 대상이 아님.
- Original ELF SHA-256: `901617de462bd1701f851c30a931be83fdd7e712e43f7da5e7b009ec85db811b`
- 교사 원본 C SHA-256 (최초 블라인드 분석 보존용): `7dc7d5ac5f9c0bc5f06033cb5df601a9284fa8531ff7c9616845163fc5859104` (추후 공개 및 검증 예정).
- **메인·CS·복습·재현·Git 원본 증거가 끝나기 전 완료 판정 금지.**

## 실행 준비 — Linux / WSL
```bash
cd /mnt/d/security-roadmap
git pull --ff-only
cd Day101-160/Day153
base64 -d target_elf.gz.b64 | gzip -dc > target_elf
chmod +x target_elf
sha256sum target_elf
file target_elf
```

## 첫 번째 독립 단계 (소스/해설을 보지 않기)
```bash
checksec --file=./target_elf  # 없으면: pwn checksec ./target_elf
strings -tx target_elf | head -60
readelf -W -h target_elf
objdump -d -M intel target_elf | less
```
1. 사용자가 `main` 후보와 입력의 도달 경로를 식별하고 주소·근거를 제출한다.
2. 입력 길이가 어디에서 만들어지고 검사되는지, 검사가 실제 복사 대상의 경계와 일치하는지 **독립 가설**을 제출한다.
3. 이후 GDB에서 레지스터·스택·복사 길이 실측을 하고, **동일 조건 ASan 비교 빌드**를 교사가 제공한다.
4. 정상·오류·경계 반례를 명령/입력으로 재현한 뒤 정확한 위치 차이를 설명한다.
5. SysV AMD64 call/ret·7~9번째 인자 추적, Day040~050 과거 Git 스택/ROP 회상→원본 대조→무힌트 반례 및 Write-up/commit 수행.

## 증거 계약
- 학생의 **실제로 실행한 명령·입력 파일·관찰 로그·환경**과 추론을 구분해 기록한다.
- 분석을 시작한 뒤 소스가 공개되더라도 최초 무힌트 분석 판단과 AI 도움 범위를 보존한다.
- `target_elf`는 위 텍스트에 포함된 원본 바이너리의 로컬 생성물이다. Git에는 복원이 가능한 `*.b64`를 보존한다.
- 패치 소스·ASan 비교 자료·replay.sh·사용자의 실제 실행 로그/Write-up은 후속 분석 후 추가한다. 현재는 **실험 준비 단계**.
