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
