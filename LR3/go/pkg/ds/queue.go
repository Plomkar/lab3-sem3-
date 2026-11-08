package ds

import "fmt"

type Queue[T comparable] struct {
	items *LinkedList[T]
}

func NewQueue[T comparable](values ...T) *Queue[T] {
	queue := &Queue[T]{items: NewLinkedList[T]()}
	for _, value := range values {
		queue.Push(value)
	}
	return queue
}

func (q *Queue[T]) list() *LinkedList[T] {
	if q.items == nil {
		q.items = NewLinkedList[T]()
	}
	return q.items
}

func (q *Queue[T]) Len() int { return q.list().Len() }

func (q *Queue[T]) Kind() string { return "Queue" }

func (q *Queue[T]) Clear() { q.list().Clear() }

func (q *Queue[T]) String() string { return q.list().String() }

func (q *Queue[T]) Push(value T) { q.list().PushBack(value) }

func (q *Queue[T]) Pop() (T, error) {
	value, err := q.list().PopFront()
	if err != nil {
		var zero T
		return zero, fmt.Errorf("%w: queue", ErrEmpty)
	}
	return value, nil
}

func (q *Queue[T]) Peek() (T, error) {
	value, err := q.list().Front()
	if err != nil {
		var zero T
		return zero, fmt.Errorf("%w: queue", ErrEmpty)
	}
	return value, nil
}

func (q *Queue[T]) ToSlice() []T { return q.list().ToSlice() }

func (q *Queue[T]) MarshalJSON() ([]byte, error) {
	return q.list().MarshalJSON()
}

func (q *Queue[T]) UnmarshalJSON(data []byte) error {
	return q.list().UnmarshalJSON(data)
}
