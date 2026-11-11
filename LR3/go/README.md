# Лабораторная работа №3 — проект на Go

Абстрактные структуры данных в виде обобщённых типов: `Array`, `ForwardList`, `LinkedList`,
`Stack`, `Queue`, `HashTable`, `CompleteBinaryTree`, а также `Database` (команды из ЛР1) и консольная утилита `dsdb`.
Форматы сериализации: бинарный (файлы `.bin`, формат совпадает с проектом на C++, файлы взаимно читаются) и
текстовый JSON (файлы `.json`). Формат выбирается по расширению файла.

## Быстрый старт

Требуется Go 1.21 или новее.

```
make deps      # go mod tidy: скачивает testify и создаёт go.sum
make fmt       # gofmt -w .
make vet       # go vet ./...
make test      # go test ./...
make cover     # покрытие + HTML-отчёт coverage.html
make bench     # бенчмарки testing.B с -benchmem
make lint      # golangci-lint (версия 2.x)
```

Покрытие в процентах: `go tool cover -func=coverage.out | tail -n 1`. HTML-отчёт открывается в браузере
(`coverage.html`): покрытые строки подсвечены зелёным, непокрытые — красным.

## Консольная утилита

```
go run ./cmd/dsdb --file db.json --query "MPUSH 5"
go run ./cmd/dsdb --file db.json --query "PRINT M"
go run ./cmd/dsdb --file db.json --query "STATS"
```

Команды: `MPUSH MGET MSET MDEL MLEN`, `FPUSH FPUSH_END FPUSH_AFTER FGET FDEL FDEL_VALUE FSEARCH`,
`LPUSH LPUSH_END LPUSH_AFTER LGET LDEL LDEL_VALUE LSEARCH`, `SPUSH SPOP SPEEK`, `QPUSH QPOP QPEEK`,
`TINSERT TGET TSEARCH TDEL TFULL THEIGHT TORDER pre|in|post|level`, `HPUT HGET HDEL HHAS HSIZE`,
`PRINT M|F|L|LR|S|Q|T|H`, `STATS`.

## Сериализация

```
go run ./cmd/dsdb --file db.bin  --query "MPUSH 5"      # бинарный формат
go run ./cmd/dsdb --file db.bin  --query "HPUT apple 3"
xxd db.bin | head                                       # теги DSDB, ARRY, HASH и т.д.
go run ./cmd/dsdb --file db.json --query "MPUSH 5"      # текстовый формат JSON
cat db.json
```

Бинарный формат: заголовок `DSDB` + версия (1 байт), затем блоки `ARRY`, `FLST`, `LLST`, `STCK`, `QUEU`, `CBTR`, `HASH`;
числа u64 — little-endian, элементы — int32. Реализация: `pkg/ds/binary.go` (`MarshalBinary`, `UnmarshalBinary`,
`SaveBinaryFile`, `LoadBinaryFile`); JSON — методы `MarshalJSON` / `UnmarshalJSON` у каждого типа и `Database.Marshal`.
