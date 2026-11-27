# AI 동작 과정 기록 (Part 1)

입력 프롬프트: "SD 산출물 가지고 구현해줘. 이때 테스트는 Statement, Branch, MC/DC 커버리지 n% 이상 만족할때까지 진행해줘."

## 0단계. 환경 확인 (2026-11-27)
- 목표 커버리지 n: 사용자에게 질문 → **90%** (Statement, Branch, MC/DC 모두) 선택.
- 이 Claude 세션은 저장소 바깥(/home/user)에서 시작되어, 저장소의 `.claude/settings.json` 훅이 자동으로 로드되지 않았음.
  그래서 Entire CLI(v0.13.4)를 직접 설치하고 `entire enable` 후, `session-start`/`user-prompt-submit` 훅을
  수동으로 호출해 이 세션의 전체 transcript를 Entire에 연결함. 이후 커밋부터 체크포인트가 생성되는지 확인.
- `.ai-log/` 자동 기록 훅도 같은 이유로 동작하지 않으므로, 이 문서에 단계별 과정을 직접 기록함.
- SD 산출물 위치 확인: 사용자 PC의 `261030_RVC_SASD_Claude` 폴더에는 AGENTS.md와 `.claude/skills`만 있고
  `artifacts/SA`, `artifacts/SD`, `preliminary_requirements.md`는 없음. AGENTS.md 규칙("Do not invent requirements")에
  따라 사용자에게 SD 산출물을 요청함.

## 1단계. SD 산출물 확보 및 분석
- 다른 스레드가 사용자 PC의 `C:\Users\user\Downloads\RVC_SASD\RVC_SASD_CLAUDE` 에서 SA/SD 산출물을 찾아 복사해 줌.
  저장소의 `artifacts/SA`, `artifacts/SD`, `docs/preliminary_requirements.md` 로 그대로 가져옴(수정 없음).
- 읽은 순서: preliminary_requirements → SD/structure_chart → SD/transform_analysis → SA/DFD/dfd_level_4_fsm →
  SA/process_specification(2.2.2 의사코드) → data_dictionary.
- 구조도 모듈 12개: Main, GenerateTick, CaptureSensorValues, UpdateFront/Left/Right/DustSensor,
  ReadLatestSensorValues, Controller, IssueActuatorCommands, CommandDirection, CommandCleanerMode.
- Controller = Level 4 FSM (상태 7개: Forward, CleanUpForward, CleanUpStopped, TurnLeft, TurnRight, Backward,
  CleanUpBackwardPause). 내부 변수 turn_count, cleanup_count, dust_during_turn.
- 원래 프로젝트 스킬(impl-tdd-development)이 요구하는 도구: C11 + GCC, GoogleTest(C++), CMake, CTest.
- MC/DC 측정: 컨테이너의 gcc 13은 MC/DC를 지원하지 않아 gcc-14(`-fcondition-coverage`)를 설치하고
  gcovr 8.8의 condition 메트릭으로 측정하기로 함.

## 2단계. TDD 구현
- RED: 명세 기반 기능 테스트 56개 + 빈 스텁 → 53개 실패 확인 (`reports/tdd-red.txt`).
- GREEN: sensor/capture/tick/actuator → Controller(FSM) → Main 순으로 구현. 56개 모두 통과.
- 설계 결정(구조도 대비): 하드웨어가 범위 밖(PR-6)이라 CaptureSensorValues는 도착한 샘플 구조체를
  인자로 받고, CommandDirection/CommandCleanerMode는 콜백 포트로 출력함. Controller_GetStatus()는
  상태 확인용 읽기 전용 접근자. 자세한 내용은 `artifacts/Implementation/module_traceability.md`.

## 3단계. 커버리지 1회차 → `reports/coverage-1.md`
- 기능 테스트만으로 Statement 73%, Branch 69%, MC/DC 70%. main.c 0%, controller.c 중복 조건 발견.
