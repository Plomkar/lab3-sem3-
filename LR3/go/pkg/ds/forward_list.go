package ds

import (
	"encoding/json"
	"fmt"
)

type forwardNode[T comparable] struct {
	value T
	next  *forwardNode[T]
}

type ForwardList[T comparable] struct {
	head *forwardNode[T]
	tail *forwardNode[T]
	size int
}

func NewForwardList[T comparable](values ...T) *ForwardList[T] {
	list := &ForwardList[T]{}
	for _, value := range values {
		list.PushBack(value)
	}
	return list
}

func (l *ForwardList[T]) Len() int { return l.size }

func (l *ForwardList[T]) Kind() string { return "ForwardList" }

func (l *ForwardList[T]) Clear() {
	l.head = nil
	l.tail = nil
	l.size = 0
}

func (l *ForwardList[T]) String() string { return joinValues(l.ToSlice()) }

func (l *ForwardList[T]) PushFront(value T) {
	node := &forwardNode[T]{value: value, next: l.head}
	l.head = node
	if l.tail == nil {
		l.tail = node
	}
	l.size++
}

func (l *ForwardList[T]) PushBack(value T) {
	node := &forwardNode[T]{value: value}
	if l.tail != nil {
		l.tail.next = node
	} else {
		l.head = node
	}
	l.tail = node
	l.size++
}

func (l *ForwardList[T]) Insert(index int, value T) error {
	if index < 0 || index > l.size {
		return indexError(index)
	}
	switch index {
	case 0:
		l.PushFront(value)
	case l.size:
		l.PushBack(value)
	default:
		prev := l.nodeAt(index - 1)
		prev.next = &forwardNode[T]{value: value, next: prev.next}
		l.size++
	}
	return nil
}

func (l *ForwardList[T]) InsertAfter(index int, value T) error {
	if err := l.checkIndex(index); err != nil {
		return err
	}
	current := l.nodeAt(index)
	node := &forwardNode[T]{value: value, next: current.next}
	current.next = node
	if current == l.tail {
		l.tail = node
	}
	l.size++
	return nil
}

func (l *ForwardList[T]) PopFront() (T, error) {
	var zero T
	if l.head == nil {
		return zero, fmt.Errorf("%w: forward list", ErrEmpty)
	}
	removed := l.head
	l.head = removed.next
	if l.head == nil {
		l.tail = nil
	}
	l.size--
	return removed.value, nil
}

func (l *ForwardList[T]) Front() (T, error) {
	if l.head == nil {
		var zero T
		return zero, fmt.Errorf("%w: forward list", ErrEmpty)
	}
	return l.head.value, nil
}

func (l *ForwardList[T]) Back() (T, error) {
	if l.tail == nil {
		var zero T
		return zero, fmt.Errorf("%w: forward list", ErrEmpty)
	}
	return l.tail.value, nil
}

func (l *ForwardList[T]) EraseAt(index int) error {
	if err := l.checkIndex(index); err != nil {
		return err
	}
	if index == 0 {
		_, err := l.PopFront()
		return err
	}
	prev := l.nodeAt(index - 1)
	removed := prev.next
	prev.next = removed.next
	if removed == l.tail {
		l.tail = prev
	}
	l.size--
	return nil
}

func (l *ForwardList[T]) EraseValue(value T) bool {
	var prev *forwardNode[T]
	for current := l.head; current != nil; prev, current = current, current.next {
		if current.value != value {
			continue
		}
		if prev != nil {
			prev.next = current.next
		} else {
			l.head = current.next
		}
		if current == l.tail {
			l.tail = prev
		}
		l.size--
		return true
	}
	return false
}

func (l *ForwardList[T]) Get(index int) (T, error) {
	if err := l.checkIndex(index); err != nil {
		var zero T
		return zero, err
	}
	return l.nodeAt(index).value, nil
}

func (l *ForwardList[T]) IndexOf(value T) (int, bool) {
	index := 0
	for current := l.head; current != nil; current = current.next {
		if current.value == value {
			return index, true
		}
		index++
	}
	return -1, false
}

func (l *ForwardList[T]) ToSlice() []T {
	items := make([]T, 0, l.size)
	for current := l.head; current != nil; current = current.next {
		items = append(items, current.value)
	}
	return items
}

func (l *ForwardList[T]) MarshalJSON() ([]byte, error) {
	return json.Marshal(l.ToSlice())
}

func (l *ForwardList[T]) UnmarshalJSON(data []byte) error {
	var items []T
	if err := json.Unmarshal(data, &items); err != nil {
		return fmt.Errorf("%w: %w", ErrFormat, err)
	}
	l.Clear()
	for _, item := range items {
		l.PushBack(item)
	}
	return nil
}

func (l *ForwardList[T]) checkIndex(index int) error {
	if index < 0 || index >= l.size {
		return indexError(index)
	}
	return nil
}

func (l *ForwardList[T]) nodeAt(index int) *forwardNode[T] {
	current := l.head
	for i := 0; i < index; i++ {
		current = current.next
	}
	return current
}
