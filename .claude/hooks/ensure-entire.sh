#!/usr/bin/env bash
# SessionStart hook: make sure Entire session capture works in this clone.
#  1. Installs the Entire CLI if it is missing (fresh Claude Code cloud containers).
#  2. Installs Entire's git hooks if they are missing (they live in .git/hooks, which a fresh clone lacks).
#     Without them, commits get no Entire-Checkpoint and nothing is recorded.
set -u
ENTIRE_VERSION="${ENTIRE_VERSION:-v0.13.4}"
input="$(cat)"
cd "${CLAUDE_PROJECT_DIR:-.}" || exit 0

installed_now=0
export GOBIN="$HOME/.local/bin"
export PATH="$GOBIN:$PATH"

if ! command -v entire >/dev/null 2>&1; then
  installed_now=1
  mkdir -p "$GOBIN"
  if command -v brew >/dev/null 2>&1; then
    brew install --cask entireio/tap/entire >&2 || true
  fi
  if ! command -v entire >/dev/null 2>&1 && command -v go >/dev/null 2>&1; then
    go install "github.com/entireio/cli/cmd/entire@${ENTIRE_VERSION}" >&2 || true
  fi
  if ! command -v entire >/dev/null 2>&1; then
    curl -fsSL https://entire.com/install.sh 2>/dev/null | bash >&2 || true
  fi
  if ! command -v entire >/dev/null 2>&1; then
    echo '{"systemMessage":"Entire CLI could not be installed automatically, so this session is NOT being recorded. Install it: https://docs.entire.com/installation"}'
    exit 0
  fi
  if [ -n "${CLAUDE_ENV_FILE:-}" ]; then
    echo "export PATH=\"$GOBIN:\$PATH\"" >> "$CLAUDE_ENV_FILE"
  fi
fi

hooks_dir="$(git rev-parse --git-path hooks 2>/dev/null || echo .git/hooks)"
if ! grep -qs entire "$hooks_dir/post-commit"; then
  # Re-runs setup non-interactively; keeps .claude/settings.json and existing git hooks.
  entire enable --agent claude-code --telemetry=false >&2 || true
fi

# Entire's own SessionStart hook ran in parallel and found no CLI, so start capture now.
if [ "$installed_now" = 1 ]; then
  printf '%s' "$input" | exec entire hooks claude-code session-start
fi
exit 0
