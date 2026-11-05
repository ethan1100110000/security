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
사용자 실행·분석·CS·복습·write-up은 아직 미완료. 시간 기준 없음.
