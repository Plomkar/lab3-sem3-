# Лабораторная работа №3 — проект на C++

Классы-контейнеры (шаблоны) в пространстве имён `ds`: `Array`, `ForwardList`, `LinkedList`, `Stack`, `Queue`,
`HashTable`, `CompleteBinaryTree`, а также `Database` (команды из ЛР1) и консольная утилита `dsdb`.
Формат сериализации — бинарный.

## Зависимости (Ubuntu / Debian)

```
sudo apt install build-essential cmake libgtest-dev libboost-test-dev catch2 cppcheck clang-tidy clang-format lcov
```

Нужен Catch2 версии 3 (в Ubuntu 24.04 пакет `catch2` — 3.x).

## Сборка и тесты

```
scripts/build_and_test.sh
```

## Покрытие кода (HTML-отчёт)

```
scripts/coverage.sh
```

Скрипт собирает проект с `--coverage`, запускает тесты и строит HTML-отчёт `coverage-report/index.html`
(`scripts/gcov_html.py` — использует только gcov и Python 3). Если установлен lcov, дополнительно создаётся отчёт
`coverage-report-lcov/index.html` (genhtml). Порог покрытия — 85 %.

## Бенчмарки

```
scripts/bench.sh            # 100000 операций
scripts/bench.sh 500000
```

## Линтеры

```
scripts/lint.sh             # cppcheck + clang-tidy
```

Компиляция с `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wold-style-cast`; сборка с `-DDS_WARNINGS_AS_ERRORS=ON`
превращает предупреждения в ошибки.

## Консольная утилита

```
./build/dsdb --file db.bin --query "MPUSH 5"
./build/dsdb --file db.bin --query "PRINT M"
./build/dsdb --file db.bin --query "STATS"
```

Команды те же, что и в Go-проекте (см. `go/README.md`).
