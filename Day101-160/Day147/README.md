# Day147 — Archive desk
2026-11-05 / W05 1/5 / 새로 준비한 자체 로컬 CTF

실행 환경: Linux x86-64, gcc. 저장소 루트에서:
```sh
sh Day101-160/Day147/build.sh
cd Day101-160/Day147
file ./archive_desk
checksec --file=./archive_desk
./archive_desk
```

오늘은 정상 입력으로 메뉴를 관찰하고, 입력 경로와 보호 기법을 도구 출력으로 설명한다. 해설과 소스를 보기 전 자신의 분석을 기록한다. 이 시점에서 exploit 완성은 완료 조건이 아니다.
checksec이 없으면 새 설치부터 하지 않고 pwndbg의 checksec 또는 readelf 자료로 확인한다.

소스와 빌드 스크립트는 재현용이며 최초 분석 전 읽지 않는 방식으로 진행한다. 소스/힌트를 보면 그 범위를 기록한다. 빌드 성공이나 교사 스모크 검증은 사용자의 분석 완료가 아니다.
CS: 정수변환·배열 가정과 반례. 복습: 이전 Pwn 풀이에서 도움받았던 단계와 실제 근거 구분.
**실제 수행: 2026-11-05~06 / Day147 / W05 1/5 완료.** 사용자 로컬에서 메뉴/입력 경로, 보호기법, Format String의 레지스터 노출과 PIE/libc 매핑, Submit의 호출자 버퍼 BOF와 Canary 실패를 GDB/역어셈블로 확인했다. 정수변환·배열 CS 및 Day085 tcache count 복습은 문답으로 완료했다. Exploit·Canary leak·ROP는 수행하지 않았다. 전체 GDB 원본과 바이너리 해시는 미보존이며 직접 검증/교사 안내/미확인을 보고서에서 구분한다.

- [Day147 write-up](day147_writeup.md)
- [사용자 관찰 출력 발췌](day147_user_observations.txt)
- 완료 진도 및 다음 회차: [progress_log.md](../../00_plan/progress_log.md). 시간 기준 없음.
