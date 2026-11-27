#!/usr/bin/env python3
"""Lists source lines whose MC/DC condition outcomes are not all covered (from gcov -g output)."""
import glob, re, sys

for path in sorted(glob.glob(sys.argv[1] + "/*.gcov")):
    src_line, src_no, name = None, None, None
    for raw in open(path, encoding="utf-8", errors="replace"):
        m = re.match(r"\s*[^:]+:\s*(\d+):(.*)", raw)
        if m and not raw.lstrip().startswith(("condition", "branch", "call")):
            src_no, src_line = int(m.group(1)), m.group(2).strip()
            if src_no == 0 and "Source:" in src_line:
                name = src_line.split("Source:")[1]
            continue
        c = re.match(r"\s*condition\s+(\d+) not covered \((\w+)\)", raw)
        if c and name and name.startswith("src/") or (c and name and "/src/" in name):
            print(f"{name.split('/src/')[-1] if '/src/' in name else name}:{src_no}: cond {c.group(1)} never {c.group(2)} | {src_line}")
