# rvc.controller

소프트웨어공학 과제 저장소.

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
