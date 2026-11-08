package ds

import (
	"encoding/json"
	"fmt"
)

type CompleteBinaryTree[T comparable] struct {
	nodes []T
}

func NewCompleteBinaryTree[T comparable](values ...T) *CompleteBinaryTree[T] {
	tree := &CompleteBinaryTree[T]{}
	for _, value := range values {
		tree.Insert(value)
	}
	return tree
}

func (t *CompleteBinaryTree[T]) Len() int { return len(t.nodes) }

func (t *CompleteBinaryTree[T]) Kind() string { return "CompleteBinaryTree" }

func (t *CompleteBinaryTree[T]) Clear() { t.nodes = t.nodes[:0] }

func (t *CompleteBinaryTree[T]) String() string { return joinValues(t.nodes) }

func (t *CompleteBinaryTree[T]) Insert(value T) {
	t.nodes = append(t.nodes, value)
}

func (t *CompleteBinaryTree[T]) Get(index int) (T, error) {
	if err := t.checkIndex(index); err != nil {
		var zero T
		return zero, err
	}
	return t.nodes[index], nil
}

func (t *CompleteBinaryTree[T]) IndexOf(value T) (int, bool) {
	for i, node := range t.nodes {
		if node == value {
			return i, true
		}
	}
	return -1, false
}

func (t *CompleteBinaryTree[T]) Erase(index int) error {
	if err := t.checkIndex(index); err != nil {
		return err
	}
	last := len(t.nodes) - 1
	t.nodes[index] = t.nodes[last]
	t.nodes = t.nodes[:last]
	return nil
}

func (t *CompleteBinaryTree[T]) Parent(index int) (int, error) {
	if err := t.checkIndex(index); err != nil {
		return -1, err
	}
	if index == 0 {
		return -1, nil
	}
	return (index - 1) / 2, nil
}

func (t *CompleteBinaryTree[T]) LeftChild(index int) (int, error) {
	if err := t.checkIndex(index); err != nil {
		return -1, err
	}
	child := 2*index + 1
	if child >= len(t.nodes) {
		return -1, nil
	}
	return child, nil
}

func (t *CompleteBinaryTree[T]) RightChild(index int) (int, error) {
	if err := t.checkIndex(index); err != nil {
		return -1, err
	}
	child := 2*index + 2
	if child >= len(t.nodes) {
		return -1, nil
	}
	return child, nil
}

func (t *CompleteBinaryTree[T]) Height() int {
	levels := 0
	for remaining := len(t.nodes); remaining > 0; remaining >>= 1 {
		levels++
	}
	return levels
}

func (t *CompleteBinaryTree[T]) IsFull() bool {
	count := len(t.nodes)
	return count&(count+1) == 0
}

func (t *CompleteBinaryTree[T]) Preorder() []T {
	result := make([]T, 0, len(t.nodes))
	t.preorderFrom(0, &result)
	return result
}

func (t *CompleteBinaryTree[T]) Inorder() []T {
	result := make([]T, 0, len(t.nodes))
	t.inorderFrom(0, &result)
	return result
}

func (t *CompleteBinaryTree[T]) Postorder() []T {
	result := make([]T, 0, len(t.nodes))
	t.postorderFrom(0, &result)
	return result
}

func (t *CompleteBinaryTree[T]) LevelOrder() []T {
	result := make([]T, len(t.nodes))
	copy(result, t.nodes)
	return result
}

func (t *CompleteBinaryTree[T]) MarshalJSON() ([]byte, error) {
	return json.Marshal(t.LevelOrder())
}

func (t *CompleteBinaryTree[T]) UnmarshalJSON(data []byte) error {
	var nodes []T
	if err := json.Unmarshal(data, &nodes); err != nil {
		return fmt.Errorf("%w: %w", ErrFormat, err)
	}
	if nodes == nil {
		nodes = []T{}
	}
	t.nodes = nodes
	return nil
}

func (t *CompleteBinaryTree[T]) checkIndex(index int) error {
	if index < 0 || index >= len(t.nodes) {
		return indexError(index)
	}
	return nil
}

func (t *CompleteBinaryTree[T]) preorderFrom(index int, out *[]T) {
	if index >= len(t.nodes) {
		return
	}
	*out = append(*out, t.nodes[index])
	t.preorderFrom(2*index+1, out)
	t.preorderFrom(2*index+2, out)
}

func (t *CompleteBinaryTree[T]) inorderFrom(index int, out *[]T) {
	if index >= len(t.nodes) {
		return
	}
	t.inorderFrom(2*index+1, out)
	*out = append(*out, t.nodes[index])
	t.inorderFrom(2*index+2, out)
}

func (t *CompleteBinaryTree[T]) postorderFrom(index int, out *[]T) {
	if index >= len(t.nodes) {
		return
	}
	t.postorderFrom(2*index+1, out)
	t.postorderFrom(2*index+2, out)
	*out = append(*out, t.nodes[index])
}
