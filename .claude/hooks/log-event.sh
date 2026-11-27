#!/usr/bin/env sh
# Records every Claude Code hook event (prompts, tool calls, results, stops) as one JSON line
# in .ai-log/<date>.jsonl, as a human-readable complement to Entire checkpoints.
# No dependencies: hook input is JSON, so newlines are stripped and it is embedded as-is.
dir="${CLAUDE_PROJECT_DIR:-.}/.ai-log"
mkdir -p "$dir" 2>/dev/null || exit 0
payload="$(cat | tr -d '\n\r')"
[ -z "$payload" ] && payload='{}'
printf '{"ts":"%s","data":%s}\n' "$(date -u +%Y-%m-%dT%H:%M:%SZ)" "$payload" >> "$dir/$(date -u +%Y-%m-%d).jsonl"
exit 0
