#!/usr/bin/env python3
"""Prints a Markdown catalog of every test, its type (by folder) and its basis (the comment above it)."""
import glob, re

KINDS = [("tests/unit/functional", "Unit - Functional (black-box, from SA/SD spec)"),
         ("tests/unit/structural", "Unit - Structural (white-box, from code decisions)"),
         ("tests/system", "System (whole-system scenarios)")]

for folder, title in KINDS:
    rows = []
    for path in sorted(glob.glob(folder + "/*.cpp")):
        comment = []
        for line in open(path, encoding="utf-8"):
            s = line.strip()
            if s.startswith("//"):
                comment.append(s.lstrip("/ ").strip())
                continue
            m = re.match(r"TEST(?:_F)?\((\w+),\s*(\w+)\)", s)
            if m:
                basis = " ".join(c for c in comment if not c.startswith("-")) or "-"
                rows.append((path.split("/")[-1], m.group(2), basis.replace("|", "/")))
            if s:
                comment = []
    print(f"## {title}: {len(rows)} tests\n")
    print("| File | Test | Basis / requirement |\n|---|---|---|")
    for f, t, b in rows:
        print(f"| {f} | {t} | {b} |")
    print()
