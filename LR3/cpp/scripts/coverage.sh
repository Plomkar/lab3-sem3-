#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

rm -rf build-cov
cmake -S . -B build-cov -DCMAKE_BUILD_TYPE=Debug -DDS_COVERAGE=ON -DDS_BUILD_BENCH=OFF
cmake --build build-cov -j
ctest --test-dir build-cov --output-on-failure

python3 scripts/gcov_html.py --build build-cov --root . --out coverage-report --fail-under 85
echo "HTML report: coverage-report/index.html"

if command -v lcov >/dev/null 2>&1 && command -v genhtml >/dev/null 2>&1; then
    lcov --capture --directory build-cov --output-file build-cov/coverage.info \
        --ignore-errors mismatch,negative,inconsistent,unused 2>/dev/null || true
    lcov --extract build-cov/coverage.info "$PWD/include/*" "$PWD/src/*" \
        --output-file build-cov/coverage.filtered.info --ignore-errors unused 2>/dev/null || true
    lcov --remove build-cov/coverage.filtered.info "$PWD/src/main.cpp" \
        --output-file build-cov/coverage.filtered.info --ignore-errors unused 2>/dev/null || true
    genhtml build-cov/coverage.filtered.info --output-directory coverage-report-lcov 2>/dev/null || true
    echo "lcov HTML report: coverage-report-lcov/index.html"
fi
