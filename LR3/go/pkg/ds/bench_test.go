package ds

import (
	"fmt"
	"testing"
)

var benchSizes = []int{1000, 100000}

func BenchmarkArrayPushBack(b *testing.B) {
	for _, size := range benchSizes {
		b.Run(fmt.Sprintf("n=%d", size), func(b *testing.B) {
			b.ReportAllocs()
			for i := 0; i < b.N; i++ {
				array := NewArray[int]()
				for j := 0; j < size; j++ {
					array.PushBack(j)
				}
			}
		})
	}
}

func BenchmarkArrayGet(b *testing.B) {
	array := NewArray[int]()
	for j := 0; j < 100000; j++ {
		array.PushBack(j)
	}
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_, _ = array.Get(i % 100000)
	}
}

func BenchmarkForwardListPushFront(b *testing.B) {
	b.ReportAllocs()
	list := NewForwardList[int]()
	for i := 0; i < b.N; i++ {
		list.PushFront(i)
	}
}

func BenchmarkForwardListPushBack(b *testing.B) {
	b.ReportAllocs()
	list := NewForwardList[int]()
	for i := 0; i < b.N; i++ {
		list.PushBack(i)
	}
}

func BenchmarkLinkedListPushBack(b *testing.B) {
	b.ReportAllocs()
	list := NewLinkedList[int]()
	for i := 0; i < b.N; i++ {
		list.PushBack(i)
	}
}

func BenchmarkStackPushPop(b *testing.B) {
	b.ReportAllocs()
	stack := NewStack[int]()
	for i := 0; i < b.N; i++ {
		stack.Push(i)
		_, _ = stack.Pop()
	}
}

func BenchmarkQueuePushPop(b *testing.B) {
	b.ReportAllocs()
	queue := NewQueue[int]()
	for i := 0; i < b.N; i++ {
		queue.Push(i)
		_, _ = queue.Pop()
	}
}

func BenchmarkTreeInsert(b *testing.B) {
	b.ReportAllocs()
	tree := NewCompleteBinaryTree[int]()
	for i := 0; i < b.N; i++ {
		tree.Insert(i)
	}
}

func BenchmarkTreeInorder(b *testing.B) {
	tree := NewCompleteBinaryTree[int]()
	for i := 0; i < 100000; i++ {
		tree.Insert(i)
	}
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_ = tree.Inorder()
	}
}

func benchmarkHashTablePut(b *testing.B, strategy ProbeStrategy) {
	keys := make([]string, 100000)
	for i := range keys {
		keys[i] = fmt.Sprintf("key%d", i*7+3)
	}
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		table := NewHashTable[int](strategy, 11)
		for j, key := range keys {
			table.Put(key, j)
		}
	}
}

func BenchmarkHashTablePutLinear(b *testing.B) {
	benchmarkHashTablePut(b, LinearProbing)
}

func BenchmarkHashTablePutDouble(b *testing.B) {
	benchmarkHashTablePut(b, DoubleHashing)
}

func benchmarkHashTableGet(b *testing.B, strategy ProbeStrategy) {
	keys := make([]string, 100000)
	table := NewHashTable[int](strategy, 11)
	for i := range keys {
		keys[i] = fmt.Sprintf("key%d", i*7+3)
		table.Put(keys[i], i)
	}
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_, _ = table.Get(keys[i%len(keys)])
	}
}

func BenchmarkHashTableGetLinear(b *testing.B) {
	benchmarkHashTableGet(b, LinearProbing)
}

func BenchmarkHashTableGetDouble(b *testing.B) {
	benchmarkHashTableGet(b, DoubleHashing)
}

func BenchmarkDatabaseMarshal(b *testing.B) {
	database := NewDatabase()
	for i := 0; i < 1000; i++ {
		database.Execute(fmt.Sprintf("MPUSH %d", i))
		database.Execute(fmt.Sprintf("QPUSH %d", i))
	}
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		data, err := database.Marshal()
		if err != nil {
			b.Fatal(err)
		}
		loaded := NewDatabase()
		if err := loaded.Unmarshal(data); err != nil {
			b.Fatal(err)
		}
	}
}
