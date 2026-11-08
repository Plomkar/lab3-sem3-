GOBIN := $(shell go env GOPATH)/bin
GOLANGCI := $(shell command -v golangci-lint 2>/dev/null || echo $(GOBIN)/golangci-lint)

.PHONY: deps fmt vet lint lint-install test race cover bench build

deps:
	go mod tidy

fmt:
	gofmt -w .

vet:
	go vet ./...

lint:
	@test -x "$(GOLANGCI)" || { echo "golangci-lint not found, run: make lint-install"; exit 1; }
	$(GOLANGCI) run ./...

lint-install:
	go install github.com/golangci/golangci-lint/v2/cmd/golangci-lint@latest

test:
	go test ./... -count=1

race:
	go test -race ./... -count=1

cover:
	go test ./pkg/... -count=1 -covermode=atomic -coverprofile=coverage.out
	go tool cover -func=coverage.out | tail -n 1
	go tool cover -html=coverage.out -o coverage.html

bench:
	go test ./pkg/ds -run "^$$" -bench . -benchmem

build:
	go build -o bin/dsdb ./cmd/dsdb
