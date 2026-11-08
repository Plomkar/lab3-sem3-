package ds

import (
	"encoding/json"
	"fmt"
)

type linkedNode[T comparable] struct {
	value T
	prev  *linkedNode[T]
	next  *linkedNode[T]
}

type LinkedList[T comparable] struct {
	head *linkedNode[T]
	tail *linkedNode[T]
	size int
}

func NewLinkedList[T comparable](values ...T) *LinkedList[T] {
	list := &LinkedList[T]{}
	for _, value := range values {
		list.PushBack(value)
	}
	return list
}

func (l *LinkedList[T]) Len() int { return l.size }

func (l *LinkedList[T]) Kind() string { return "LinkedList" }

func (l *LinkedList[T]) Clear() {
	l.head = nil
	l.tail = nil
	l.size = 0
}

func (l *LinkedList[T]) String() string { return joinValues(l.ToSlice()) }

func (l *LinkedList[T]) ReversedString() string { return joinValues(l.ToReversedSlice()) }

func (l *LinkedList[T]) PushFront(value T) {
	node := &linkedNode[T]{value: value, next: l.head}
	if l.head != nil {
		l.head.prev = node
	} else {
		l.tail = node
	}
	l.head = node
	l.size++
}

func (l *LinkedList[T]) PushBack(value T) {
	node := &linkedNode[T]{value: value, prev: l.tail}
	if l.tail != nil {
		l.tail.next = node
	} else {
		l.head = node
	}
	l.tail = node
	l.size++
}

func (l *LinkedList[T]) Insert(index int, value T) error {
	if index < 0 || index > l.size {
		return indexError(index)
	}
	switch index {
	case 0:
		l.PushFront(value)
	case l.size:
		l.PushBack(value)
	default:
		next := l.nodeAt(index)
		prev := next.prev
		node := &linkedNode[T]{value: value, prev: prev, next: next}
		prev.next = node
		next.prev = node
		l.size++
	}
	return nil
}

func (l *LinkedList[T]) InsertAfter(index int, value T) error {
	if err := l.checkIndex(index); err != nil {
		return err
	}
	current := l.nodeAt(index)
	if current == l.tail {
		l.PushBack(value)
		return nil
	}
	next := current.next
	node := &linkedNode[T]{value: value, prev: current, next: next}
	current.next = node
	next.prev = node
	l.size++
	return nil
}

func (l *LinkedList[T]) PopFront() (T, error) {
	if l.head == nil {
		var zero T
		return zero, fmt.Errorf("%w: linked list", ErrEmpty)
	}
	value := l.head.value
	l.unlink(l.head)
	return value, nil
}

func (l *LinkedList[T]) PopBack() (T, error) {
	if l.tail == nil {
		var zero T
		return zero, fmt.Errorf("%w: linked list", ErrEmpty)
	}
	value := l.tail.value
	l.unlink(l.tail)
	return value, nil
}

func (l *LinkedList[T]) Front() (T, error) {
	if l.head == nil {
		var zero T
		return zero, fmt.Errorf("%w: linked list", ErrEmpty)
	}
	return l.head.value, nil
}

func (l *LinkedList[T]) Back() (T, error) {
	if l.tail == nil {
		var zero T
		return zero, fmt.Errorf("%w: linked list", ErrEmpty)
	}
	return l.tail.value, nil
}

func (l *LinkedList[T]) EraseAt(index int) error {
	if err := l.checkIndex(index); err != nil {
		return err
	}
	l.unlink(l.nodeAt(index))
	return nil
}

func (l *LinkedList[T]) EraseValue(value T) bool {
	for current := l.head; current != nil; current = current.next {
		if current.value == value {
			l.unlink(current)
			return true
		}
	}
	return false
}

func (l *LinkedList[T]) Get(index int) (T, error) {
	if err := l.checkIndex(index); err != nil {
		var zero T
		return zero, err
	}
	return l.nodeAt(index).value, nil
}

func (l *LinkedList[T]) IndexOf(value T) (int, bool) {
	index := 0
	for current := l.head; current != nil; current = current.next {
		if current.value == value {
			return index, true
		}
		index++
	}
	return -1, false
}

func (l *LinkedList[T]) ToSlice() []T {
	items := make([]T, 0, l.size)
	for current := l.head; current != nil; current = current.next {
		items = append(items, current.value)
	}
	return items
}

func (l *LinkedList[T]) ToReversedSlice() []T {
	items := make([]T, 0, l.size)
	for current := l.tail; current != nil; current = current.prev {
		items = append(items, current.value)
	}
	return items
}

func (l *LinkedList[T]) MarshalJSON() ([]byte, error) {
	return json.Marshal(l.ToSlice())
}

func (l *LinkedList[T]) UnmarshalJSON(data []byte) error {
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

func (l *LinkedList[T]) checkIndex(index int) error {
	if index < 0 || index >= l.size {
		return indexError(index)
	}
	return nil
}

func (l *LinkedList[T]) nodeAt(index int) *linkedNode[T] {
	if index < l.size/2 {
		current := l.head
		for i := 0; i < index; i++ {
			current = current.next
		}
		return current
	}
	current := l.tail
	for i := l.size - 1; i > index; i-- {
		current = current.prev
	}
	return current
}

func (l *LinkedList[T]) unlink(node *linkedNode[T]) {
	if node.prev != nil {
		node.prev.next = node.next
	} else {
		l.head = node.next
	}
	if node.next != nil {
		node.next.prev = node.prev
	} else {
		l.tail = node.prev
	}
	l.size--
}
