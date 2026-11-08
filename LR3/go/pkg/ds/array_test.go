package ds

import (
	"encoding/json"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestArrayNewIsEmpty(t *testing.T) {
	array := NewArray[int]()
	assert.Equal(t, 0, array.Len())
	assert.True(t, IsEmpty(array))
	assert.Equal(t, "Array", array.Kind())
	assert.Equal(t, "", array.String())
}

func TestArrayPushBackAndGet(t *testing.T) {
	array := NewArray[int]()
	for i := 0; i < 10; i++ {
		array.PushBack(i)
	}
	assert.Equal(t, 10, array.Len())
	value, err := array.Get(9)
	require.NoError(t, err)
	assert.Equal(t, 9, value)
	assert.Equal(t, "0 1 2 3 4 5 6 7 8 9", array.String())
}

func TestArrayInitialValues(t *testing.T) {
	values := []int{5, 4, 3, 8, 17}
	array := NewArray(values...)
	values[0] = 100
	assert.Equal(t, "5 4 3 8 17", array.String())
}

func TestArrayInsert(t *testing.T) {
	array := NewArray(1, 3)
	require.NoError(t, array.Insert(1, 2))
	require.NoError(t, array.Insert(0, 0))
	require.NoError(t, array.Insert(4, 4))
	assert.Equal(t, "0 1 2 3 4", array.String())
}

func TestArrayInsertOutOfRange(t *testing.T) {
	array := NewArray(1)
	for _, index := range []int{-1, 2, 100} {
		err := array.Insert(index, 5)
		assert.ErrorIs(t, err, ErrIndexOutOfRange)
	}
	assert.Equal(t, 1, array.Len())
}

func TestArrayGetSet(t *testing.T) {
	array := NewArray(1, 2, 3)
	require.NoError(t, array.Set(1, 20))
	value, err := array.Get(1)
	require.NoError(t, err)
	assert.Equal(t, 20, value)

	_, err = array.Get(3)
	assert.ErrorIs(t, err, ErrIndexOutOfRange)
	_, err = array.Get(-1)
	assert.ErrorIs(t, err, ErrIndexOutOfRange)
	assert.ErrorIs(t, array.Set(3, 1), ErrIndexOutOfRange)
	assert.ErrorIs(t, array.Set(-1, 1), ErrIndexOutOfRange)
}

func TestArrayErase(t *testing.T) {
	array := NewArray(5, 4, 3, 8, 17)
	require.NoError(t, array.Erase(2))
	assert.Equal(t, "5 4 8 17", array.String())
	require.NoError(t, array.Erase(3))
	assert.Equal(t, "5 4 8", array.String())
	assert.ErrorIs(t, array.Erase(3), ErrIndexOutOfRange)
	assert.ErrorIs(t, array.Erase(-1), ErrIndexOutOfRange)
}

func TestArrayClearAndClone(t *testing.T) {
	array := NewArray(1, 2, 3)
	clone := array.Clone()
	array.Clear()
	assert.Equal(t, 0, array.Len())
	assert.Equal(t, "1 2 3", clone.String())
	require.NoError(t, clone.Set(0, 100))
	assert.Equal(t, 0, array.Len())
}

func TestArrayToSliceReturnsCopy(t *testing.T) {
	array := NewArray(1, 2, 3)
	items := array.ToSlice()
	items[0] = 100
	value, err := array.Get(0)
	require.NoError(t, err)
	assert.Equal(t, 1, value)
}

func TestArrayWorksWithStrings(t *testing.T) {
	array := NewArray("a", "b")
	array.PushBack("c")
	assert.Equal(t, "a b c", array.String())
}

func TestArrayJSONRoundTrip(t *testing.T) {
	source := NewArray(10, 20, 30)
	data, err := json.Marshal(source)
	require.NoError(t, err)
	assert.JSONEq(t, "[10,20,30]", string(data))

	var loaded Array[int]
	require.NoError(t, json.Unmarshal(data, &loaded))
	assert.Equal(t, []int{10, 20, 30}, loaded.ToSlice())
}

func TestArrayJSONEmptyAndNull(t *testing.T) {
	data, err := json.Marshal(NewArray[int]())
	require.NoError(t, err)
	assert.JSONEq(t, "[]", string(data))

	loaded := NewArray(1, 2)
	require.NoError(t, json.Unmarshal([]byte("null"), loaded))
	assert.Equal(t, 0, loaded.Len())
}

func TestArrayJSONRejectsInvalidData(t *testing.T) {
	var loaded Array[int]
	err := loaded.UnmarshalJSON([]byte(`{"not":"an array"}`))
	assert.ErrorIs(t, err, ErrFormat)
}
