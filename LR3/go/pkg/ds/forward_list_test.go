package ds

import (
	"encoding/json"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestForwardListNewIsEmpty(t *testing.T) {
	list := NewForwardList[int]()
	assert.True(t, IsEmpty(list))
	assert.Equal(t, "ForwardList", list.Kind())
	_, err := list.Front()
	assert.ErrorIs(t, err, ErrEmpty)
	_, err = list.Back()
	assert.ErrorIs(t, err, ErrEmpty)
	_, err = list.PopFront()
	assert.ErrorIs(t, err, ErrEmpty)
}

func TestForwardListPushFrontAndBack(t *testing.T) {
	list := NewForwardList[int]()
	list.PushFront(2)
	list.PushFront(1)
	list.PushBack(3)
	assert.Equal(t, "1 2 3", list.String())
	front, err := list.Front()
	require.NoError(t, err)
	assert.Equal(t, 1, front)
	back, err := list.Back()
	require.NoError(t, err)
	assert.Equal(t, 3, back)
}

func TestForwardListInsert(t *testing.T) {
	list := NewForwardList(1, 4)
	require.NoError(t, list.Insert(1, 2))
	require.NoError(t, list.Insert(2, 3))
	require.NoError(t, list.Insert(0, 0))
	require.NoError(t, list.Insert(5, 5))
	assert.Equal(t, "0 1 2 3 4 5", list.String())
	assert.ErrorIs(t, list.Insert(10, 1), ErrIndexOutOfRange)
	assert.ErrorIs(t, list.Insert(-1, 1), ErrIndexOutOfRange)
}

func TestForwardListInsertAfter(t *testing.T) {
	list := NewForwardList(1, 3)
	require.NoError(t, list.InsertAfter(0, 2))
	require.NoError(t, list.InsertAfter(2, 4))
	assert.Equal(t, "1 2 3 4", list.String())
	back, err := list.Back()
	require.NoError(t, err)
	assert.Equal(t, 4, back)
	assert.ErrorIs(t, list.InsertAfter(4, 1), ErrIndexOutOfRange)
}

func TestForwardListPopFront(t *testing.T) {
	list := NewForwardList(1, 2)
	value, err := list.PopFront()
	require.NoError(t, err)
	assert.Equal(t, 1, value)
	value, err = list.PopFront()
	require.NoError(t, err)
	assert.Equal(t, 2, value)
	assert.True(t, IsEmpty(list))
	list.PushBack(7)
	back, err := list.Back()
	require.NoError(t, err)
	assert.Equal(t, 7, back)
}

func TestForwardListEraseAt(t *testing.T) {
	list := NewForwardList(1, 2, 3, 4, 5)
	require.NoError(t, list.EraseAt(0))
	require.NoError(t, list.EraseAt(1))
	require.NoError(t, list.EraseAt(2))
	assert.Equal(t, "2 4", list.String())
	back, err := list.Back()
	require.NoError(t, err)
	assert.Equal(t, 4, back)
	assert.ErrorIs(t, list.EraseAt(2), ErrIndexOutOfRange)
	list.PushBack(6)
	assert.Equal(t, "2 4 6", list.String())
}

func TestForwardListEraseValue(t *testing.T) {
	list := NewForwardList(1, 2, 3, 2)
	assert.True(t, list.EraseValue(2))
	assert.Equal(t, "1 3 2", list.String())
	assert.True(t, list.EraseValue(1))
	assert.True(t, list.EraseValue(2))
	back, err := list.Back()
	require.NoError(t, err)
	assert.Equal(t, 3, back)
	assert.False(t, list.EraseValue(42))
	assert.True(t, list.EraseValue(3))
	assert.True(t, IsEmpty(list))
}

func TestForwardListGetAndIndexOf(t *testing.T) {
	list := NewForwardList(10, 20, 30)
	value, err := list.Get(1)
	require.NoError(t, err)
	assert.Equal(t, 20, value)
	_, err = list.Get(3)
	assert.ErrorIs(t, err, ErrIndexOutOfRange)

	index, found := list.IndexOf(30)
	assert.True(t, found)
	assert.Equal(t, 2, index)
	index, found = list.IndexOf(99)
	assert.False(t, found)
	assert.Equal(t, -1, index)
}

func TestForwardListClear(t *testing.T) {
	list := NewForwardList(1, 2, 3)
	list.Clear()
	assert.True(t, IsEmpty(list))
	assert.Equal(t, "", list.String())
	assert.Empty(t, list.ToSlice())
}

func TestForwardListLongList(t *testing.T) {
	list := NewForwardList[int]()
	for i := 0; i < 100000; i++ {
		list.PushBack(i)
	}
	assert.Equal(t, 100000, list.Len())
}

func TestForwardListJSONRoundTrip(t *testing.T) {
	source := NewForwardList(7, 8, 9)
	data, err := json.Marshal(source)
	require.NoError(t, err)
	assert.JSONEq(t, "[7,8,9]", string(data))

	loaded := NewForwardList(1)
	require.NoError(t, json.Unmarshal(data, loaded))
	assert.Equal(t, []int{7, 8, 9}, loaded.ToSlice())
}

func TestForwardListJSONRejectsInvalidData(t *testing.T) {
	list := NewForwardList[int]()
	assert.ErrorIs(t, list.UnmarshalJSON([]byte(`"text"`)), ErrFormat)
}
