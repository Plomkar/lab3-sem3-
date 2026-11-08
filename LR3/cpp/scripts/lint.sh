#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

cppcheck --enable=warning,style,performance,portability --std=c++17 --inline-suppr \
    --suppress=missingIncludeSystem --suppress=noExplicitConstructor --error-exitcode=1 \
    -Iinclude include src bench

cmake -S . -B build-lint -DCMAKE_BUILD_TYPE=Debug -DDS_BUILD_TESTS=OFF -DDS_BUILD_BENCH=OFF >/dev/null

clang-tidy -p build-lint \
    --header-filter="^$PWD/(include|src|bench)/" \
    src/*.cpp 2>&1 \
  | grep -E 'warning:|error:' \
  || true