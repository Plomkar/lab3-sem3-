package ds

import (
	"encoding/json"
	"fmt"
)

type Array[T any] struct {
	items []T
}

func NewArray[T any](values ...T) *Array[T] {
	items := make([]T, len(values))
	copy(items, values)
	return &Array[T]{items: items}
}

func (a *Array[T]) Len() int { return len(a.items) }

func (a *Array[T]) Clear() { a.items = a.items[:0] }

func (a *Array[T]) Kind() string { return "Array" }

func (a *Array[T]) String() string { return joinValues(a.items) }

func (a *Array[T]) PushBack(value T) {
	a.items = append(a.items, value)
}

func (a *Array[T]) Insert(index int, value T) error {
	if index < 0 || index > len(a.items) {
		return indexError(index)
	}
	var zero T
	a.items = append(a.items, zero)
	copy(a.items[index+1:], a.items[index:])
	a.items[index] = value
	return nil
}

func (a *Array[T]) Get(index int) (T, error) {
	if index < 0 || index >= len(a.items) {
		var zero T
		return zero, indexError(index)
	}
	return a.items[index], nil
}

func (a *Array[T]) Set(index int, value T) error {
	if index < 0 || index >= len(a.items) {
		return indexError(index)
	}
	a.items[index] = value
	return nil
}

func (a *Array[T]) Erase(index int) error {
	if index < 0 || index >= len(a.items) {
		return indexError(index)
	}
	a.items = append(a.items[:index], a.items[index+1:]...)
	return nil
}

func (a *Array[T]) ToSlice() []T {
	items := make([]T, len(a.items))
	copy(items, a.items)
	return items
}

func (a *Array[T]) Clone() *Array[T] {
	return NewArray(a.items...)
}

func (a *Array[T]) MarshalJSON() ([]byte, error) {
	return json.Marshal(a.ToSlice())
}

func (a *Array[T]) UnmarshalJSON(data []byte) error {
	var items []T
	if err := json.Unmarshal(data, &items); err != nil {
		return fmt.Errorf("%w: %w", ErrFormat, err)
	}
	if items == nil {
		items = []T{}
	}
	a.items = items
	return nil
}
