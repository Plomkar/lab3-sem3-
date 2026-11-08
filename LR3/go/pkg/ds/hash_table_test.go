package ds

import (
	"encoding/json"
	"fmt"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestHashFunctions(t *testing.T) {
	assert.Equal(t, uint64(0), polynomialHash(""))
	assert.Equal(t, uint64(1677554), polynomialHash("abc"))
	assert.NotEqual(t, polynomialHash("abc"), polynomialHash("abd"))
	assert.Equal(t, uint64(14695981039346656037), fnv1aHash(""))
	assert.Equal(t, uint64(12638187200555641996), fnv1aHash("a"))
	assert.NotEqual(t, fnv1aHash("a"), fnv1aHash("b"))
}

func TestPrimes(t *testing.T) {
	assert.False(t, isPrime(0))
	assert.False(t, isPrime(1))
	assert.True(t, isPrime(2))
	assert.True(t, isPrime(13))
	assert.False(t, isPrime(15))
	assert.True(t, isPrime(1009))
	assert.Equal(t, 2, nextPrime(0))
	assert.Equal(t, 11, nextPrime(11))
	assert.Equal(t, 13, nextPrime(12))
	assert.Equal(t, 1009, nextPrime(1000))
}

func TestProbeStrategyString(t *testing.T) {
	assert.Equal(t, "linear", LinearProbing.String())
	assert.Equal(t, "double", DoubleHashing.String())
}

func TestHashTableNewIsEmpty(t *testing.T) {
	table := NewHashTable[int](DoubleHashing, 11)
	assert.True(t, IsEmpty(table))
	assert.Equal(t, "HashTable", table.Kind())
	assert.Equal(t, 11, table.Capacity())
	assert.Equal(t, DoubleHashing, table.Strategy())
	_, found := table.Get("missing")
	assert.False(t, found)
}

func TestHashTableCapacityIsPrime(t *testing.T) {
	assert.Equal(t, 23, NewHashTable[int](LinearProbing, 20).Capacity())
	assert.Equal(t, 3, NewHashTable[int](LinearProbing, 0).Capacity())
}

func TestHashTablePutGetUpdate(t *testing.T) {
	table := NewHashTable[int](DoubleHashing, 11)
	table.Put("a", 1)
	table.Put("b", 2)
	table.Put("a", 10)
	assert.Equal(t, 2, table.Len())
	value, found := table.Get("a")
	assert.True(t, found)
	assert.Equal(t, 10, value)
	assert.True(t, table.Contains("b"))
	assert.False(t, table.Contains("c"))
}

func TestHashTableEraseAndReinsert(t *testing.T) {
	table := NewHashTable[int](LinearProbing, 11)
	table.Put("x", 1)
	assert.True(t, table.Erase("x"))
	assert.False(t, table.Erase("x"))
	assert.False(t, table.Contains("x"))
	assert.Equal(t, 0, table.Len())
	table.Put("x", 2)
	value, found := table.Get("x")
	assert.True(t, found)
	assert.Equal(t, 2, value)
}

func TestHashTableKeysAndString(t *testing.T) {
	table := NewHashTable[int](DoubleHashing, 11)
	table.Put("b", 2)
	table.Put("a", 1)
	table.Put("c", 3)
	assert.Equal(t, []string{"a", "b", "c"}, table.Keys())
	assert.Equal(t, "a=1 b=2 c=3", table.String())
}

func TestHashTableClear(t *testing.T) {
	table := NewHashTable[int](DoubleHashing, 11)
	table.Put("a", 1)
	table.Put("b", 2)
	table.Clear()
	assert.True(t, IsEmpty(table))
	assert.False(t, table.Contains("a"))
	assert.InDelta(t, 0.0, table.LoadFactor(), 1e-9)
	table.Put("a", 5)
	value, found := table.Get("a")
	assert.True(t, found)
	assert.Equal(t, 5, value)
}

func TestHashTableZeroValueIsUsable(t *testing.T) {
	var table HashTable[int]
	assert.InDelta(t, 0.0, table.LoadFactor(), 1e-9)
	table.Put("a", 1)
	value, found := table.Get("a")
	assert.True(t, found)
	assert.Equal(t, 1, value)
}

func TestHashTableGrowth(t *testing.T) {
	for _, strategy := range []ProbeStrategy{LinearProbing, DoubleHashing} {
		table := NewHashTable[int](strategy, 5)
		initial := table.Capacity()
		for i := 0; i < 500; i++ {
			table.Put(fmt.Sprintf("key%d", i), i)
		}
		assert.Equal(t, 500, table.Len())
		assert.Greater(t, table.Capacity(), initial)
		assert.LessOrEqual(t, table.LoadFactor(), 0.7+1e-9)
		for i := 0; i < 500; i++ {
			value, found := table.Get(fmt.Sprintf("key%d", i))
			require.True(t, found)
			assert.Equal(t, i, value)
		}
	}
}

func TestHashTableTombstonesAreCleaned(t *testing.T) {
	table := NewHashTable[int](DoubleHashing, 11)
	for round := 0; round < 50; round++ {
		for i := 0; i < 5; i++ {
			table.Put(fmt.Sprintf("t%d", i), round)
		}
		for i := 0; i < 5; i++ {
			assert.True(t, table.Erase(fmt.Sprintf("t%d", i)))
		}
		table.Put(fmt.Sprintf("r%d", round), round)
	}
	assert.Equal(t, 50, table.Len())
	for round := 0; round < 50; round++ {
		value, found := table.Get(fmt.Sprintf("r%d", round))
		require.True(t, found)
		assert.Equal(t, round, value)
	}
}

func TestHashTableReusesErasedSlots(t *testing.T) {
	table := NewHashTable[int](LinearProbing, 11)
	table.Put("a", 1)
	table.Put("b", 2)
	assert.True(t, table.Erase("a"))
	table.Put("a", 3)
	value, found := table.Get("a")
	assert.True(t, found)
	assert.Equal(t, 3, value)
	assert.Equal(t, 2, table.Len())
}

func TestHashTableStrategiesAgree(t *testing.T) {
	linear := NewHashTable[int](LinearProbing, 7)
	double := NewHashTable[int](DoubleHashing, 7)
	keys := make(map[string]bool)
	for i := 0; i < 100; i++ {
		key := fmt.Sprintf("collide%d", i%37)
		keys[key] = true
		linear.Put(key, i)
		double.Put(key, i)
	}
	assert.Equal(t, len(keys), linear.Len())
	assert.Equal(t, len(keys), double.Len())
	for key := range keys {
		left, _ := linear.Get(key)
		right, _ := double.Get(key)
		assert.Equal(t, left, right)
	}
}

func TestHashTableJSONRoundTrip(t *testing.T) {
	table := NewHashTable[int](LinearProbing, 11)
	for i := 0; i < 40; i++ {
		table.Put(fmt.Sprintf("k%d", i), i)
	}
	table.Erase("k7")

	data, err := json.Marshal(table)
	require.NoError(t, err)

	loaded := NewHashTable[int](DoubleHashing, 11)
	loaded.Put("old", 1)
	require.NoError(t, json.Unmarshal(data, loaded))
	assert.Equal(t, LinearProbing, loaded.Strategy())
	assert.Equal(t, 39, loaded.Len())
	assert.False(t, loaded.Contains("old"))
	assert.False(t, loaded.Contains("k7"))
	value, found := loaded.Get("k39")
	assert.True(t, found)
	assert.Equal(t, 39, value)
}

func TestHashTableJSONRejectsInvalidData(t *testing.T) {
	table := NewHashTable[int](DoubleHashing, 11)
	assert.ErrorIs(t, table.UnmarshalJSON([]byte(`[1,2,3]`)), ErrFormat)
	assert.ErrorIs(t, table.UnmarshalJSON([]byte(`{"strategy":"cuckoo","entries":{}}`)), ErrFormat)
}
