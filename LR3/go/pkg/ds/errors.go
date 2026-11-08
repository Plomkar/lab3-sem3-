package ds

import (
	"errors"
	"fmt"
	"strings"
)

var (
	ErrIndexOutOfRange = errors.New("index out of range")
	ErrEmpty           = errors.New("container is empty")
	ErrFormat          = errors.New("invalid data format")
)

func indexError(index int) error {
	return fmt.Errorf("%w: %d", ErrIndexOutOfRange, index)
}

func joinValues[T any](items []T) string {
	parts := make([]string, len(items))
	for i, item := range items {
		parts[i] = fmt.Sprint(item)
	}
	return strings.Join(parts, " ")
}
