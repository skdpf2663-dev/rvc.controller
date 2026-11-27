# AI 동작 과정 기록 규칙 (소프트웨어공학 과제 Part 1)

이 저장소는 Entire CLI와 Claude Code 훅으로 AI 에이전트의 모든 작업을 기록합니다.

## 기록 방식
- **Entire 체크포인트**: 커밋할 때마다 그 사이의 프롬프트, 전체 대화(transcript), 수정한 파일이
  `refs/entire/checkpoints/...` 에 저장되고 커밋 메시지에 `Entire-Checkpoint:` 트레일러가 붙습니다.
  확인: `entire checkpoint list`, `entire checkpoint explain <id>`
- **.ai-log/**: `.claude/hooks/log-event.sh` 가 프롬프트, 모든 도구 호출과 결과, 종료 이벤트를
  날짜별 JSONL로 남깁니다. 커밋에 함께 포함합니다.

## 에이전트 작업 규칙
1. 의미 있는 단계마다 작게 커밋합니다 (예: 설계 분석, 구현 1개 모듈, 테스트 추가, 커버리지 측정).
   커밋이 곧 체크포인트이므로 한 번에 몰아서 커밋하지 않습니다.
2. 커밋 메시지에 단계 종류를 붙입니다: `impl:`, `test(unit-functional):`, `test(unit-structural):`,
   `test(system):`, `coverage:`, `fix:`, `docs:`.
3. 테스트는 종류별 폴더에 둡니다.
   - `tests/unit/functional/` : 명세(SD 산출물) 기반 블랙박스 단위 테스트
   - `tests/unit/structural/` : 코드 구조(분기, 조건) 기반 화이트박스 단위 테스트
   - `tests/system/` : 시스템 전체 시나리오 테스트
4. 커버리지(Statement, Branch, MC/DC)를 측정할 때마다 결과를 `reports/coverage-<n>.md` 에 남기고
   목표 n%에 도달할 때까지 반복합니다. 각 반복에서 무엇을 보고 어떤 테스트를 추가했는지 적습니다.
5. Entire 설정을 끄거나 지우는 명령(`entire disable`, `entire clean`)은 사용자 허락 없이 실행하지 않습니다.
