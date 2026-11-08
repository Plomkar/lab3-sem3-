package ds

import (
	"encoding/json"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestStackNewIsEmpty(t *testing.T) {
	stack := NewStack[int]()
	assert.True(t, IsEmpty(stack))
	assert.Equal(t, "Stack", stack.Kind())
	_, err := stack.Pop()
	assert.ErrorIs(t, err, ErrEmpty)
	_, err = stack.Peek()
	assert.ErrorIs(t, err, ErrEmpty)
}

func TestStackLifoOrder(t *testing.T) {
	stack := NewStack(1, 2, 3)
	assert.Equal(t, "3 2 1", stack.String())
	peeked, err := stack.Peek()
	require.NoError(t, err)
	assert.Equal(t, 3, peeked)
	assert.Equal(t, 3, stack.Len())
	for _, expected := range []int{3, 2, 1} {
		value, popErr := stack.Pop()
		require.NoError(t, popErr)
		assert.Equal(t, expected, value)
	}
	assert.True(t, IsEmpty(stack))
}

func TestStackClearAndToSlice(t *testing.T) {
	stack := NewStack(1, 2)
	assert.Equal(t, []int{2, 1}, stack.ToSlice())
	stack.Clear()
	assert.True(t, IsEmpty(stack))
}

func TestStackZeroValueIsUsable(t *testing.T) {
	var stack Stack[int]
	assert.Equal(t, 0, stack.Len())
	stack.Push(5)
	value, err := stack.Pop()
	require.NoError(t, err)
	assert.Equal(t, 5, value)
}

func TestStackJSONRoundTrip(t *testing.T) {
	source := NewStack(1, 2, 3)
	data, err := json.Marshal(source)
	require.NoError(t, err)
	assert.JSONEq(t, "[3,2,1]", string(data))

	var loaded Stack[int]
	require.NoError(t, json.Unmarshal(data, &loaded))
	value, err := loaded.Pop()
	require.NoError(t, err)
	assert.Equal(t, 3, value)
	assert.Equal(t, 2, loaded.Len())
}

func TestStackJSONRejectsInvalidData(t *testing.T) {
	var stack Stack[int]
	assert.ErrorIs(t, stack.UnmarshalJSON([]byte(`"x"`)), ErrFormat)
}

func TestQueueNewIsEmpty(t *testing.T) {
	queue := NewQueue[int]()
	assert.True(t, IsEmpty(queue))
	assert.Equal(t, "Queue", queue.Kind())
	_, err := queue.Pop()
	assert.ErrorIs(t, err, ErrEmpty)
	_, err = queue.Peek()
	assert.ErrorIs(t, err, ErrEmpty)
}

func TestQueueFifoOrder(t *testing.T) {
	queue := NewQueue(1, 2, 3)
	assert.Equal(t, "1 2 3", queue.String())
	peeked, err := queue.Peek()
	require.NoError(t, err)
	assert.Equal(t, 1, peeked)
	for _, expected := range []int{1, 2, 3} {
		value, popErr := queue.Pop()
		require.NoError(t, popErr)
		assert.Equal(t, expected, value)
	}
	assert.True(t, IsEmpty(queue))
}

func TestQueueReuseAfterDrain(t *testing.T) {
	queue := NewQueue(1, 2)
	_, err := queue.Pop()
	require.NoError(t, err)
	_, err = queue.Pop()
	require.NoError(t, err)
	queue.Push(9)
	value, err := queue.Peek()
	require.NoError(t, err)
	assert.Equal(t, 9, value)
}

func TestQueueClearAndToSlice(t *testing.T) {
	queue := NewQueue(1, 2)
	assert.Equal(t, []int{1, 2}, queue.ToSlice())
	queue.Clear()
	assert.True(t, IsEmpty(queue))
}

func TestQueueZeroValueIsUsable(t *testing.T) {
	var queue Queue[int]
	assert.Equal(t, 0, queue.Len())
	queue.Push(5)
	value, err := queue.Pop()
	require.NoError(t, err)
	assert.Equal(t, 5, value)
}

func TestQueueJSONRoundTrip(t *testing.T) {
	source := NewQueue(1, 2, 3)
	data, err := json.Marshal(source)
	require.NoError(t, err)
	assert.JSONEq(t, "[1,2,3]", string(data))

	var loaded Queue[int]
	require.NoError(t, json.Unmarshal(data, &loaded))
	assert.Equal(t, "1 2 3", loaded.String())
}

func TestQueueJSONRejectsInvalidData(t *testing.T) {
	var queue Queue[int]
	assert.ErrorIs(t, queue.UnmarshalJSON([]byte(`"x"`)), ErrFormat)
}
