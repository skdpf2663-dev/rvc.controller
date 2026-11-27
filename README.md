# rvc.controller

소프트웨어공학 과제 저장소. 로봇청소기(RVC) 자동 청소 제어기를 SD 산출물로부터 C11로 구현하고,
Statement / Branch / MC/DC 커버리지 90% 이상이 될 때까지 테스트를 만든 과정입니다.

## 결과 요약

| 항목 | 위치 |
| --- | --- |
| 입력 산출물 (SA/SD, 요구사항) | `artifacts/SA`, `artifacts/SD`, `docs/preliminary_requirements.md` |
| 구현 (C11) | `include/`, `src/` |
| 테스트 97개 (GoogleTest) | `tests/unit/functional` 56, `tests/unit/structural` 20, `tests/system` 21 |
| 커버리지 반복 기록 | `reports/coverage-1.md` → `coverage-2.md` → `coverage-3.md` |
| 최종 커버리지 | Statement 98.8%, Branch 99.2%, MC/DC 100% |
| 테스트 분류표, 모듈 추적표 | `artifacts/Implementation/test_report.md`, `module_traceability.md` |
| AI 동작 과정 기록 | `docs/ai-process-log.md`, `docs/entire-checkpoints/`, 커밋별 `Entire-Checkpoint:` |

### 빌드와 테스트
```bash
# GCC 14 이상 필요 (MC/DC 측정용 -fcondition-coverage), GoogleTest, CMake, gcovr
cmake -S . -B build -DCMAKE_C_COMPILER=gcc-14 -DCMAKE_CXX_COMPILER=g++-14
cmake --build build && (cd build && ctest)
scripts/coverage.sh                 # 전체 커버리지 (Statement=line, Branch, MC/DC=condition)
scripts/coverage.sh unit_functional # 종류별
printf '1 0 0 0\n- - - -\n' | build/rvc   # 실행 파일: 한 줄 = 1 tick, Front Left Right Dust (0/1/-)
```

## AI 작업 기록 (Entire + Claude Code 훅)

이 저장소에서 Claude Code가 하는 모든 구현과 테스트 과정은 자동으로 기록됩니다. 규칙은 [CLAUDE.md](CLAUDE.md)에 있습니다.

| 기록 | 위치 | 보는 법 |
| --- | --- | --- |
| Entire 체크포인트 (프롬프트, 전체 대화, 수정 파일) | `refs/entire/checkpoints/*` (커밋마다 `Entire-Checkpoint:` 트레일러) | `entire checkpoint list`, `entire checkpoint explain <id>` |
| 이벤트 로그 (프롬프트, 도구 호출과 결과) | `.ai-log/<날짜>.jsonl` | 파일을 직접 열기 |

### 내 컴퓨터에서 확인하기
```bash
brew install --cask entireio/tap/entire     # 또는 curl -fsSL https://entire.com/install.sh | bash
git clone https://github.com/skdpf2663-dev/rvc.controller && cd rvc.controller
git fetch origin 'refs/entire/*:refs/entire/*'
entire enable --agent claude-code           # 이 클론에서 다시 기록을 켤 때
entire checkpoint list
```

### 훅 구성 (`.claude/settings.json`)
- Entire 기본 훅 10개: `entire enable --agent claude-code`가 설치
- `.claude/hooks/ensure-entire.sh` (SessionStart): Entire CLI와 git 훅이 없으면 설치해서 새 환경에서도 기록이 끊기지 않게 함
- `.claude/hooks/log-event.sh` (모든 이벤트): `.ai-log/`에 시간순 JSONL 기록
