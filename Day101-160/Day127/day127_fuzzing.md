# Day127 — TinyXML-2 trial fuzz run

## 대상과 실행 조건

* 대상: TinyXML-2 (`8224e427b655b83dae5e2298f1e6919523a78737`)
* 하네스: Day126 CLI wrapper의 파일 입력 방식 (`@@`), AFL++ 계측·ASan·UBSan·non-PIE 빌드
* 대상 바이너리 SHA-256: `15fa7a6dc5627c5a51f3e0c68eafcf17ec763ecad8de666ad376a2cd3c761b4c`
* 초기 입력: Day123 XML seed 12개
* 실행: `afl-fuzz -m none -V 60 -i Day101-160/Day123/corpus_raw -o Day101-160/Day127/out_cli_trial -- Day101-160/Day126/day126_cli_afl @@`
* WSL의 `core_pattern` 검사 우회를 위해 `AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1`을 사용했다.

## 관찰 결과

60초 동안 5,503회 실행해 평균 91.69 exec/s를 기록했다. 시작 seed 12개 외에 새 corpus 입력 52개가 보관되어 최종 `corpus_count`는 64개였다. `corpus_favored`는 10개, `edges_found`는 738, `bitmap_cvg`는 1.13%, `stability`는 100%였다. 저장된 crash와 hang은 각각 0개였다. `-V 60` 제한에 도달해 퍼저가 정상 종료했다.

## 해석과 한계

새 corpus 52개는 AFL++가 보관한 입력 수이며 TinyXML-2 내부에서 서로 다른 소스 경로 52개를 찾았다는 뜻은 아니다. `bitmap_cvg=1.13%` 역시 계측 맵의 사용 비율이지 소스 코드 coverage 비율이 아니다. 일부 초기 seed는 새 계측 결과를 추가하지 못한다는 경고가 있었지만, 이 결과만으로 해당 XML 파일을 삭제하지는 않는다.

이번 trial에서 저장된 crash는 0개다. 다만 실행 시간이 짧고, 하네스에서 도달한 경로에 검사가 한정되며, `core_pattern` 검사 우회 옵션을 사용했으므로 취약점이 없다고 결론 내릴 수 없다. 이후 crash 후보가 나오면 해당 입력을 단독 재현하고 sanitizer 또는 GDB 증거로 원인을 확인한다.
