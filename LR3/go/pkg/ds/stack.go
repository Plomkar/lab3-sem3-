package ds

import "fmt"

type Stack[T comparable] struct {
	items *ForwardList[T]
}

func NewStack[T comparable](values ...T) *Stack[T] {
	stack := &Stack[T]{items: NewForwardList[T]()}
	for _, value := range values {
		stack.Push(value)
	}
	return stack
}

func (s *Stack[T]) list() *ForwardList[T] {
	if s.items == nil {
		s.items = NewForwardList[T]()
	}
	return s.items
}

func (s *Stack[T]) Len() int { return s.list().Len() }

func (s *Stack[T]) Kind() string { return "Stack" }

func (s *Stack[T]) Clear() { s.list().Clear() }

func (s *Stack[T]) String() string { return s.list().String() }

func (s *Stack[T]) Push(value T) { s.list().PushFront(value) }

func (s *Stack[T]) Pop() (T, error) {
	value, err := s.list().PopFront()
	if err != nil {
		var zero T
		return zero, fmt.Errorf("%w: stack", ErrEmpty)
	}
	return value, nil
}

func (s *Stack[T]) Peek() (T, error) {
	value, err := s.list().Front()
	if err != nil {
		var zero T
		return zero, fmt.Errorf("%w: stack", ErrEmpty)
	}
	return value, nil
}

func (s *Stack[T]) ToSlice() []T { return s.list().ToSlice() }

func (s *Stack[T]) MarshalJSON() ([]byte, error) {
	return s.list().MarshalJSON()
}

func (s *Stack[T]) UnmarshalJSON(data []byte) error {
	return s.list().UnmarshalJSON(data)
}
