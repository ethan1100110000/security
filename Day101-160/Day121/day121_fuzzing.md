# Day121 - Open-source fuzzing target 선정

## 1. 목표

Day121의 목표는 이후 AFL++ 프로젝트에서 사용할 작고 빌드 가능한 C/C++ 오픈소스 타깃 후보 3개를 비교하고, 현재 학습 범위에 맞는 하나를 선정하는 것이다.

선정할 때 취약점이 많아 보이는지만 판단하지 않는다. 정해진 기간 안에 빌드, 계측, harness 작성, 반복 실행, crash 재현과 root cause 분석까지 수행할 수 있어야 한다.

## 2. 선정 기준

### Buildability

- 현재 WSL 환경에서 Clang, AFL++와 sanitizer로 빌드할 수 있어야 한다.
- 의존성과 빌드 시간이 과도하지 않아야 한다.
- 분석할 소스 범위가 지나치게 크지 않아야 한다.
- 동일한 결과를 재현할 수 있도록 대상 commit을 고정할 수 있어야 한다.

### Attack surface

- 외부에서 받은 입력을 실제로 파싱해야 한다.
- 단순 복사나 출력보다 분기, 상태 변화와 길이 처리가 충분해야 한다.
- 정상 입력을 seed로 사용했을 때 parser의 깊은 경로에 도달할 수 있어야 한다.
- 메모리 입력 API가 있으면 파일 I/O 없이 raw bytes와 실제 길이를 직접 전달할 수 있어 harness 작성과 persistent mode 적용에 유리하다.

### Project fit

공격 표면이 넓은 것은 fuzzing 타깃의 장점이지만, 타깃이 지나치게 크면 harness 작성과 crash triage 비용이 커진다. 이번 프로젝트에서는 buildability와 attack surface 사이의 균형을 우선한다.

## 3. 후보 비교

| 순위 | 후보 | Buildability | Attack surface 및 입력 API | 판단 |
|---:|---|---|---|---|
| 1 | TinyXML-2 | C++로 작성됐으며 핵심 소스가 `tinyxml2.cpp`, `tinyxml2.h` 두 파일이고 CMake를 지원한다. | `XMLDocument::Parse(const char *xml, size_t nBytes)`로 메모리 입력이 가능하다. 태그, 속성, 텍스트, 중첩 구조와 엔티티 처리 등 여러 XML 파싱 경로가 있다. | 빌드 난도와 파싱 복잡성이 모두 중간 수준이어서 이번 프로젝트에 가장 적합하다고 판단했다. |
| 2 | cJSON | ANSI C로 작성됐고 CMake와 Makefile을 지원해 세 후보 중 시작 난도가 가장 낮다. | `cJSON_ParseWithLengthOpts(data, size, ...)` 형태의 길이 기반 메모리 입력 API가 있다. JSON 객체, 배열, 문자열, 숫자와 중첩을 파싱한다. | 빌드와 실행은 쉽지만 공식 저장소에 fuzz harness가 이미 있어 독립적인 harness 설계 연습이 줄어든다. TinyXML-2 빌드가 불가능할 때 사용할 예비 후보로 둔다. |
| 3 | LibYAML | CMake를 지원하지만 reader, scanner, parser, loader 등 여러 C 소스로 구성된다. | `yaml_parser_set_input_string(input, size)`로 메모리 입력이 가능하고 YAML 문법과 parser 상태로 인해 공격 표면이 넓다. | 넓은 공격 표면은 장점이지만 parser 초기화, 이벤트 반복, 자원 해제와 crash triage 비용이 현재 프로젝트 범위에는 크다고 판단했다. |

## 4. 최종 선정

- Target: TinyXML-2
- Repository: https://github.com/leethomason/tinyxml2
- Branch: `master`
- Pinned commit: `8224e427b655b83dae5e2298f1e6919523a78737`
- Implementation language: C++
- Input format: XML
- Memory input API: `tinyxml2::XMLDocument::Parse(const char *xml, size_t nBytes)`
- Normal compiler: `clang++`
- AFL++ compiler: `afl-clang-fast++`

다음 명령으로 원격 `master`의 commit을 확인했다.

```bash
git ls-remote https://github.com/leethomason/tinyxml2.git refs/heads/master
```

확인 결과:

```text
8224e427b655b83dae5e2298f1e6919523a78737        refs/heads/master
```

따라서 조사한 저장소 상태와 로컬에서 확인한 원격 branch 상태가 일치한다. 이 명령은 원격 commit 식별만 증명하며 실제 로컬 빌드 성공은 증명하지 않는다.

## 5. TinyXML-2와 XML 입력의 관계

이번 프로젝트에서 분석하고 계측하는 대상은 TinyXML-2의 C++ parser 코드다. AFL++가 변이하는 대상은 그 parser에 전달되는 XML 바이트다.

```text
AFL++가 XML 입력을 변이
→ harness가 입력 주소와 실제 크기를 전달
→ TinyXML-2의 Parse()가 XML을 해석
→ ASan/UBSan이 C++ 코드의 오류를 탐지
```

XML은 실행할 프로그램 코드가 아니라 C++ parser가 읽는 구조화된 입력 데이터다. Day114 toy parser에서 Magic, Version, Command와 Length 형식을 맞춰야 깊은 `memcpy` 경로에 도달했던 것처럼, TinyXML-2에서도 최소한의 XML 문법을 갖춘 seed가 태그, 속성, 중첩과 특수문자 처리 경로에 도달하는 데 유리하다.

필요한 XML 지식은 정상 seed를 만들고 변이가 어느 parser 경로에 영향을 주는지 설명할 수 있는 정도로 제한한다. 전체 XML 표준이나 웹 개발을 별도로 학습하는 것이 이번 프로젝트의 목적은 아니다.

## 6. 예상 harness 방향

향후 harness는 AFL++가 제공한 raw bytes와 실제 입력 길이를 받아 다음과 같은 흐름으로 `Parse()`에 전달한다.

```cpp
tinyxml2::XMLDocument document;
document.Parse(reinterpret_cast<const char *>(data), size);
```

세부 harness 구현, 객체 생명주기, 입력 크기 제한과 persistent mode 적용은 이후 Day에서 검증한다. Day121에서는 입력 API가 존재하고 직접 호출 가능한지만 선정 근거로 사용한다.

## 7. 전환 조건

다음 Day에서 고정 commit을 clean checkout한 뒤 아래 문제가 계속되면 cJSON으로 전환한다.

- 일반 Clang 빌드와 `afl-clang-fast++` 빌드가 모두 안정적으로 만들어지지 않는다.
- 오류 원인을 기록하고 합리적인 수정 한 번을 시도해도 해결되지 않는다.
- 메모리 입력 API를 사용하려면 TinyXML-2 본체를 크게 수정해야 한다.
- 정상 XML도 sanitizer 빌드에서 결정적으로 실행되지 않는다.
- 문제 해결에 Day122 실습 시간의 절반 이상이 소모된다.

CMake나 컴파일러가 단순히 설치되지 않은 것은 타깃의 buildability 실패가 아니라 로컬 환경 준비 문제로 구분한다.

## 8. 한계

- 현재 단계에서는 CMake 설정, 소스 수와 공개 API를 확인했을 뿐 실제 WSL 빌드를 완료하지 않았다.
- TinyXML-2는 유지보수되고 있는 성숙한 parser이므로 짧은 fuzzing에서 crash가 발견된다고 보장할 수 없다.
- crash가 없더라도 harness, instrumentation, corpus, coverage와 재현 절차를 검증하는 것이 프로젝트의 학습 결과가 될 수 있다.
- XML 문법을 모르면 대부분의 입력이 parser 초기에 거부될 수 있으므로 seed 제작 전 최소 문법을 확인해야 한다.

## 9. Day122 계획

1. TinyXML-2 저장소를 clone하고 pinned commit을 checkout한다.
2. compiler, CMake와 target commit 정보를 기록한다.
3. 일반 `clang++` 빌드를 수행한다.
4. ASan/UBSan 빌드를 수행한다.
5. 정상 XML과 비정상 XML을 각각 실행해 parser가 결정적으로 동작하는지 확인한다.
6. 빌드 명령, 결과와 실패 사례를 `day122_fuzzing.md`에 기록한다.
