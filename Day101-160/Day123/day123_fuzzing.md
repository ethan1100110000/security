# Day123 - TinyXML-2 Real Seed Corpus

## Target

- Project: TinyXML-2
- Commit: 8224e427b655b83dae5e2298f1e6919523a78737
- Corpus directory: corpus_raw/
- Seed count: 12

## Seed classification

| File | Validity | Syntax feature | Expected result | Size | Selection reason |

|---|---|---|---|---:|---|

| 01_minimal.xml | valid | 빈 루트 요소 | 파싱 성공 예상 | 7 | 최소 정상 파싱 경로의 기준 |
| 02_text.xml | valid | 텍스트를 포함한 루트 | 파싱 성공 예상| 18 | 텍스트 노드 |
| 03_attribute.xml | valid | 속성을 가진 빈 요소 | 파싱 성공 예상 | 14 | 속성 이름 따옴표 속성값 처리 경로|
| 04_nested.xml | valid | 중첩 구조 | 파싱 성공 예상 | 32 | 중첩구조 |
| 05_siblings.xml | valid | 형제 요소 구조 | 파싱 성공 예상 | 21 | 형제 요소 |
| 06_comment.xml | valid | 주석 처리 | 파싱 성공 예상 | 31 | 주석 |
| 07_cdata.xml | valid | CDATA | 파싱 성공 예상 | 34 | CDATA |
| 08_entity.xml | valid | 엔티티 참조 | 파싱 성공 예상 | 29 | 엔티티 인식과 특수문자 변환 경로 |
| 09_declaration.xml | valid | xml선언을 포함한 입력 | 파싱 성공 예상 | 45 | xml선언 |
| 10_bad_close.xml | invalid | 닫는 태그 불일치 | 파싱 실패 예상 | 20 | 태그 불일치 |
| 11_bad_attribute.xml | invalid | 속성값의 따옴표 누락 | 파싱 실패 예상 | 12 | 속성값 따옴표 누락 |
| 12_unclosed_comment.xml | invalid | 종료되지 않은 주석 | 파싱 실패 예상 | 21 | 종료되지 않은 주석 |


## Corpus curation decision

- 12개 seed의 SHA-256이 모두 달라 바이트 단위의 완전 중복은 확인되지 않았다.
- 서로 다른 파일도 같은 파싱 경로나 coverage를 만들 수 있으므로 semantic·coverage 중복이 없다고 확정할 수는 없다.
- `01_minimal.xml`은 최소 정상 입력의 기준이므로 현재 유지한다.
- Day124에서 input harness를 작성한 뒤 `afl-showmap`과 `afl-cmin`으로 coverage 기반 중복을 다시 판단한다.
