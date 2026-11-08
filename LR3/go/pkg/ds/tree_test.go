package ds

import (
	"encoding/json"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestTreeNewIsEmpty(t *testing.T) {
	tree := NewCompleteBinaryTree[int]()
	assert.True(t, IsEmpty(tree))
	assert.Equal(t, 0, tree.Height())
	assert.True(t, tree.IsFull())
	assert.Equal(t, "CompleteBinaryTree", tree.Kind())
	assert.Equal(t, "", tree.String())
}

func TestTreeInsertAndGet(t *testing.T) {
	tree := NewCompleteBinaryTree(1, 2, 3, 4, 5)
	assert.Equal(t, 5, tree.Len())
	assert.Equal(t, "1 2 3 4 5", tree.String())
	value, err := tree.Get(4)
	require.NoError(t, err)
	assert.Equal(t, 5, value)
	_, err = tree.Get(5)
	assert.ErrorIs(t, err, ErrIndexOutOfRange)
	_, err = tree.Get(-1)
	assert.ErrorIs(t, err, ErrIndexOutOfRange)
}

func TestTreeIndexOf(t *testing.T) {
	tree := NewCompleteBinaryTree(7, 8, 9)
	index, found := tree.IndexOf(9)
	assert.True(t, found)
	assert.Equal(t, 2, index)
	index, found = tree.IndexOf(100)
	assert.False(t, found)
	assert.Equal(t, -1, index)
}

func TestTreeHeight(t *testing.T) {
	cases := map[int]int{1: 1, 2: 2, 3: 2, 4: 3, 7: 3, 8: 4}
	for count, expected := range cases {
		tree := NewCompleteBinaryTree[int]()
		for i := 0; i < count; i++ {
			tree.Insert(i)
		}
		assert.Equal(t, expected, tree.Height(), "count=%d", count)
	}
}

func TestTreeIsFull(t *testing.T) {
	cases := map[int]bool{1: true, 2: false, 3: true, 5: false, 7: true}
	for count, expected := range cases {
		tree := NewCompleteBinaryTree[int]()
		for i := 0; i < count; i++ {
			tree.Insert(i)
		}
		assert.Equal(t, expected, tree.IsFull(), "count=%d", count)
	}
}

func TestTreeRelatives(t *testing.T) {
	tree := NewCompleteBinaryTree(1, 2, 3, 4, 5)

	parent, err := tree.Parent(0)
	require.NoError(t, err)
	assert.Equal(t, -1, parent)
	parent, err = tree.Parent(4)
	require.NoError(t, err)
	assert.Equal(t, 1, parent)

	left, err := tree.LeftChild(1)
	require.NoError(t, err)
	assert.Equal(t, 3, left)
	right, err := tree.RightChild(1)
	require.NoError(t, err)
	assert.Equal(t, 4, right)

	left, err = tree.LeftChild(2)
	require.NoError(t, err)
	assert.Equal(t, -1, left)
	right, err = tree.RightChild(2)
	require.NoError(t, err)
	assert.Equal(t, -1, right)

	_, err = tree.Parent(5)
	assert.ErrorIs(t, err, ErrIndexOutOfRange)
	_, err = tree.LeftChild(5)
	assert.ErrorIs(t, err, ErrIndexOutOfRange)
	_, err = tree.RightChild(5)
	assert.ErrorIs(t, err, ErrIndexOutOfRange)
}

func TestTreeTraversals(t *testing.T) {
	tree := NewCompleteBinaryTree(1, 2, 3, 4, 5, 6, 7)
	assert.Equal(t, []int{1, 2, 4, 5, 3, 6, 7}, tree.Preorder())
	assert.Equal(t, []int{4, 2, 5, 1, 6, 3, 7}, tree.Inorder())
	assert.Equal(t, []int{4, 5, 2, 6, 7, 3, 1}, tree.Postorder())
	assert.Equal(t, []int{1, 2, 3, 4, 5, 6, 7}, tree.LevelOrder())
}

func TestTreeTraversalsOfEmptyTree(t *testing.T) {
	tree := NewCompleteBinaryTree[int]()
	assert.Empty(t, tree.Preorder())
	assert.Empty(t, tree.Inorder())
	assert.Empty(t, tree.Postorder())
	assert.Empty(t, tree.LevelOrder())
}

func TestTreeEraseKeepsTreeComplete(t *testing.T) {
	tree := NewCompleteBinaryTree(1, 2, 3, 4, 5)
	require.NoError(t, tree.Erase(1))
	assert.Equal(t, "1 5 3 4", tree.String())
	require.NoError(t, tree.Erase(3))
	assert.Equal(t, "1 5 3", tree.String())
	assert.ErrorIs(t, tree.Erase(3), ErrIndexOutOfRange)
	assert.ErrorIs(t, tree.Erase(-1), ErrIndexOutOfRange)
	require.NoError(t, tree.Erase(0))
	require.NoError(t, tree.Erase(0))
	require.NoError(t, tree.Erase(0))
	assert.True(t, IsEmpty(tree))
}

func TestTreeClear(t *testing.T) {
	tree := NewCompleteBinaryTree(1, 2, 3)
	tree.Clear()
	assert.True(t, IsEmpty(tree))
}

func TestTreeJSONRoundTrip(t *testing.T) {
	source := NewCompleteBinaryTree(9, 8, 7, 6)
	data, err := json.Marshal(source)
	require.NoError(t, err)
	assert.JSONEq(t, "[9,8,7,6]", string(data))

	var loaded CompleteBinaryTree[int]
	require.NoError(t, json.Unmarshal(data, &loaded))
	assert.Equal(t, "9 8 7 6", loaded.String())
}

func TestTreeJSONNullAndInvalid(t *testing.T) {
	loaded := NewCompleteBinaryTree(1, 2)
	require.NoError(t, json.Unmarshal([]byte("null"), loaded))
	assert.Equal(t, 0, loaded.Len())
	assert.ErrorIs(t, loaded.UnmarshalJSON([]byte(`"x"`)), ErrFormat)
}

func TestTreeWorksWithFloats(t *testing.T) {
	tree := NewCompleteBinaryTree(1.5, 2.5)
	assert.Equal(t, "1.5 2.5", tree.String())
}
