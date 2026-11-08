package ds

import (
	"encoding/json"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestLinkedListNewIsEmpty(t *testing.T) {
	list := NewLinkedList[int]()
	assert.True(t, IsEmpty(list))
	assert.Equal(t, "LinkedList", list.Kind())
	_, err := list.Front()
	assert.ErrorIs(t, err, ErrEmpty)
	_, err = list.Back()
	assert.ErrorIs(t, err, ErrEmpty)
	_, err = list.PopFront()
	assert.ErrorIs(t, err, ErrEmpty)
	_, err = list.PopBack()
	assert.ErrorIs(t, err, ErrEmpty)
}

func TestLinkedListPushFrontAndBack(t *testing.T) {
	list := NewLinkedList[int]()
	list.PushBack(2)
	list.PushFront(1)
	list.PushBack(3)
	assert.Equal(t, "1 2 3", list.String())
	assert.Equal(t, "3 2 1", list.ReversedString())
	front, err := list.Front()
	require.NoError(t, err)
	assert.Equal(t, 1, front)
	back, err := list.Back()
	require.NoError(t, err)
	assert.Equal(t, 3, back)
}

func TestLinkedListInsert(t *testing.T) {
	list := NewLinkedList(1, 4)
	require.NoError(t, list.Insert(1, 2))
	require.NoError(t, list.Insert(2, 3))
	require.NoError(t, list.Insert(0, 0))
	require.NoError(t, list.Insert(5, 5))
	assert.Equal(t, "0 1 2 3 4 5", list.String())
	assert.Equal(t, "5 4 3 2 1 0", list.ReversedString())
	assert.ErrorIs(t, list.Insert(10, 1), ErrIndexOutOfRange)
	assert.ErrorIs(t, list.Insert(-1, 1), ErrIndexOutOfRange)
}

func TestLinkedListInsertAfter(t *testing.T) {
	list := NewLinkedList(1, 3)
	require.NoError(t, list.InsertAfter(0, 2))
	require.NoError(t, list.InsertAfter(2, 4))
	assert.Equal(t, "1 2 3 4", list.String())
	assert.Equal(t, "4 3 2 1", list.ReversedString())
	assert.ErrorIs(t, list.InsertAfter(4, 1), ErrIndexOutOfRange)
}

func TestLinkedListPopFrontAndBack(t *testing.T) {
	list := NewLinkedList(1, 2, 3)
	value, err := list.PopFront()
	require.NoError(t, err)
	assert.Equal(t, 1, value)
	value, err = list.PopBack()
	require.NoError(t, err)
	assert.Equal(t, 3, value)
	value, err = list.PopBack()
	require.NoError(t, err)
	assert.Equal(t, 2, value)
	assert.True(t, IsEmpty(list))
	list.PushFront(5)
	back, err := list.Back()
	require.NoError(t, err)
	assert.Equal(t, 5, back)
}

func TestLinkedListEraseAt(t *testing.T) {
	list := NewLinkedList(1, 2, 3, 4, 5)
	require.NoError(t, list.EraseAt(0))
	require.NoError(t, list.EraseAt(3))
	require.NoError(t, list.EraseAt(1))
	assert.Equal(t, "2 4", list.String())
	assert.Equal(t, "4 2", list.ReversedString())
	assert.ErrorIs(t, list.EraseAt(2), ErrIndexOutOfRange)
}

func TestLinkedListEraseValue(t *testing.T) {
	list := NewLinkedList(1, 2, 3, 2)
	assert.True(t, list.EraseValue(2))
	assert.Equal(t, "1 3 2", list.String())
	assert.False(t, list.EraseValue(42))
	assert.True(t, list.EraseValue(2))
	back, err := list.Back()
	require.NoError(t, err)
	assert.Equal(t, 3, back)
}

func TestLinkedListGetFromBothHalves(t *testing.T) {
	list := NewLinkedList(0, 1, 2, 3, 4, 5, 6, 7)
	for index := 0; index < 8; index++ {
		value, err := list.Get(index)
		require.NoError(t, err)
		assert.Equal(t, index, value)
	}
	_, err := list.Get(8)
	assert.ErrorIs(t, err, ErrIndexOutOfRange)
	_, err = list.Get(-1)
	assert.ErrorIs(t, err, ErrIndexOutOfRange)
}

func TestLinkedListIndexOf(t *testing.T) {
	list := NewLinkedList(10, 20, 30)
	index, found := list.IndexOf(20)
	assert.True(t, found)
	assert.Equal(t, 1, index)
	index, found = list.IndexOf(99)
	assert.False(t, found)
	assert.Equal(t, -1, index)
}

func TestLinkedListClear(t *testing.T) {
	list := NewLinkedList(1, 2, 3)
	list.Clear()
	assert.True(t, IsEmpty(list))
	assert.Equal(t, "", list.ReversedString())
}

func TestLinkedListJSONRoundTrip(t *testing.T) {
	source := NewLinkedList(7, 8, 9)
	data, err := json.Marshal(source)
	require.NoError(t, err)
	assert.JSONEq(t, "[7,8,9]", string(data))

	loaded := NewLinkedList(1)
	require.NoError(t, json.Unmarshal(data, loaded))
	assert.Equal(t, "7 8 9", loaded.String())
	assert.Equal(t, "9 8 7", loaded.ReversedString())
}

func TestLinkedListJSONRejectsInvalidData(t *testing.T) {
	list := NewLinkedList[int]()
	assert.ErrorIs(t, list.UnmarshalJSON([]byte(`"text"`)), ErrFormat)
}
