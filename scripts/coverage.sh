#!/usr/bin/env bash
# Builds with coverage instrumentation (GCC 14: --coverage -fcondition-coverage),
# runs the selected CTest suites, and prints Statement (line), Branch, and
# MC/DC (gcov condition coverage) for the production C code in src/.
# Usage: scripts/coverage.sh [ctest -R regex]   e.g. scripts/coverage.sh unit_functional
set -euo pipefail
cd "$(dirname "$0")/.."
FILTER="${1:-.}"
BUILD=build-cov
cmake -S . -B "$BUILD" -DRVC_COVERAGE=ON -DCMAKE_C_COMPILER=gcc-14 -DCMAKE_CXX_COMPILER=g++-14 >/dev/null
cmake --build "$BUILD" -j8 >/dev/null
find "$BUILD" -name '*.gcda' -delete
(cd "$BUILD" && ctest -R "$FILTER" --output-on-failure | tail -3)
COMMON=(--root . --object-directory "$BUILD" --filter 'src/' --gcov-executable gcov-14)
mkdir -p "$BUILD/report"
gcovr "${COMMON[@]}" --json "$BUILD/report/cov.json" >/dev/null
for m in line branch condition; do
  echo "== $m =="
  gcovr --add-tracefile "$BUILD/report/cov.json" --txt-metric "$m" 2>/dev/null | sed -n '/^File/,$p'
done
# Detailed MC/DC: gcov -g (--conditions) lists each condition outcome not yet shown independent.
mkdir -p "$BUILD/gcov"
( cd "$BUILD/gcov" && for f in ../CMakeFiles/rvc_core.dir/src/*.gcda ../CMakeFiles/rvc.dir/src/*.gcda; do
    [ -f "$f" ] && gcov-14 -g -b -o "$(dirname "$f")" "$f" >/dev/null 2>&1 || true; done )
echo "== conditions not covered (gcov -g) =="
python3 scripts/mcdc_gaps.py "$BUILD/gcov"
