# Day128 — Crash collection과 재현 준비

## 후보 수집

Day127의 60초 TinyXML-2 CLI 퍼징 결과를 확인했다.
`crashes/`와 `hangs/`에서 입력 파일이 확인되지 않아 현재 triage queue의 후보는 각각 0건이다.

## 재현 절차

`replay_candidate.sh`는 입력 파일 하나를 Day126 AFL 빌드와 Debug 빌드에
각각 파일 인자로 전달한다. 입력 크기·SHA-256, 두 바이너리의 SHA-256,
각 실행의 종료 코드·표준출력·표준에러를 별도로 기록한다.

정상 XML과 문법 오류 XML로 스크립트를 시험했다. 두 입력 모두 두 빌드에서
exit=0이었다. XML 문법 오류는 파서의 오류 반환이며 crash 후보로 분류하지 않는다.

## 해석과 한계

이번 실행에서 수집된 crash와 hang 후보는 0건이다. 이 결과만으로
TinyXML-2에 취약점이 없다고 판단할 수 없다. Day127 실행은 60초였고,
WSL의 core_pattern 검사를 우회하는 옵션도 사용했다. 이후 후보가 생기면
동일 입력의 단독 재현 결과와 sanitizer 또는 GDB 근거를 확인한 뒤 분류한다.