package ds

import (
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"strconv"
	"strings"
)

const snapshotVersion = 1

type snapshot struct {
	Version     int                      `json:"version"`
	Array       *Array[int]              `json:"array"`
	ForwardList *ForwardList[int]        `json:"forward_list"`
	LinkedList  *LinkedList[int]         `json:"linked_list"`
	Stack       *Stack[int]              `json:"stack"`
	Queue       *Queue[int]              `json:"queue"`
	Tree        *CompleteBinaryTree[int] `json:"tree"`
	HashTable   *HashTable[int]          `json:"hash_table"`
}

type handler func(args []string) (string, error)

type Database struct {
	array       *Array[int]
	forwardList *ForwardList[int]
	linkedList  *LinkedList[int]
	stack       *Stack[int]
	queue       *Queue[int]
	tree        *CompleteBinaryTree[int]
	hashTable   *HashTable[int]
	handlers    map[string]handler
}

func NewDatabase() *Database {
	database := &Database{
		array:       NewArray[int](),
		forwardList: NewForwardList[int](),
		linkedList:  NewLinkedList[int](),
		stack:       NewStack[int](),
		queue:       NewQueue[int](),
		tree:        NewCompleteBinaryTree[int](),
		hashTable:   NewHashTable[int](DoubleHashing, 11),
		handlers:    make(map[string]handler),
	}
	database.registerHandlers()
	return database
}

func (d *Database) Array() *Array[int]             { return d.array }
func (d *Database) ForwardList() *ForwardList[int] { return d.forwardList }
func (d *Database) LinkedList() *LinkedList[int]   { return d.linkedList }
func (d *Database) Stack() *Stack[int]             { return d.stack }
func (d *Database) Queue() *Queue[int]             { return d.queue }
func (d *Database) Tree() *CompleteBinaryTree[int] { return d.tree }
func (d *Database) HashTable() *HashTable[int]     { return d.hashTable }

func (d *Database) Containers() []Container {
	return []Container{d.array, d.forwardList, d.linkedList, d.stack, d.queue, d.tree, d.hashTable}
}

func (d *Database) Execute(query string) string {
	fields := strings.Fields(query)
	if len(fields) == 0 {
		return "ERROR: empty query"
	}
	run, ok := d.handlers[fields[0]]
	if !ok {
		return "Unknown command: " + fields[0]
	}
	reply, err := run(fields[1:])
	if err != nil {
		return "ERROR: " + err.Error()
	}
	return reply
}

func (d *Database) Marshal() ([]byte, error) {
	return json.MarshalIndent(snapshot{
		Version:     snapshotVersion,
		Array:       d.array,
		ForwardList: d.forwardList,
		LinkedList:  d.linkedList,
		Stack:       d.stack,
		Queue:       d.queue,
		Tree:        d.tree,
		HashTable:   d.hashTable,
	}, "", "  ")
}

func (d *Database) Unmarshal(data []byte) error {
	var snap snapshot
	if err := json.Unmarshal(data, &snap); err != nil {
		return fmt.Errorf("%w: %w", ErrFormat, err)
	}
	if snap.Version != snapshotVersion {
		return fmt.Errorf("%w: unsupported version %d", ErrFormat, snap.Version)
	}
	if snap.Array == nil || snap.ForwardList == nil || snap.LinkedList == nil || snap.Stack == nil ||
		snap.Queue == nil || snap.Tree == nil || snap.HashTable == nil {
		return fmt.Errorf("%w: missing section", ErrFormat)
	}
	d.array = snap.Array
	d.forwardList = snap.ForwardList
	d.linkedList = snap.LinkedList
	d.stack = snap.Stack
	d.queue = snap.Queue
	d.tree = snap.Tree
	d.hashTable = snap.HashTable
	return nil
}

func (d *Database) SaveToFile(path string) error {
	data, err := d.Marshal()
	if err != nil {
		return err
	}
	return os.WriteFile(path, data, 0o600)
}

func (d *Database) LoadFromFile(path string) error {
	data, err := os.ReadFile(filepath.Clean(path))
	if err != nil {
		return err
	}
	return d.Unmarshal(data)
}

func argInt(args []string, position int) (int, error) {
	if position >= len(args) {
		return 0, fmt.Errorf("%w: integer argument expected", ErrFormat)
	}
	value, err := strconv.Atoi(args[position])
	if err != nil {
		return 0, fmt.Errorf("%w: invalid integer %q", ErrFormat, args[position])
	}
	return value, nil
}

func argKey(args []string, position int) (string, error) {
	if position >= len(args) {
		return "", fmt.Errorf("%w: key argument expected", ErrFormat)
	}
	return args[position], nil
}

func indexOrMinusOne(index int, found bool) string {
	if !found {
		return "-1"
	}
	return strconv.Itoa(index)
}

func boolText(value bool) string {
	if value {
		return "TRUE"
	}
	return "FALSE"
}

func (d *Database) registerHandlers() {
	d.registerArray()
	d.registerForwardList()
	d.registerLinkedList()
	d.registerStack()
	d.registerQueue()
	d.registerTree()
	d.registerHashTable()
	d.registerCommon()
}

func (d *Database) registerArray() {
	d.handlers["MPUSH"] = func(args []string) (string, error) {
		first, err := argInt(args, 0)
		if err != nil {
			return "", err
		}
		if len(args) < 2 {
			d.array.PushBack(first)
			return strconv.Itoa(first), nil
		}
		second, err := argInt(args, 1)
		if err != nil {
			return "", err
		}
		if err := d.array.Insert(first, second); err != nil {
			return "", err
		}
		return strconv.Itoa(second), nil
	}
	d.handlers["MGET"] = func(args []string) (string, error) {
		index, err := argInt(args, 0)
		if err != nil {
			return "", err
		}
		value, err := d.array.Get(index)
		if err != nil {
			return "", err
		}
		return strconv.Itoa(value), nil
	}
	d.handlers["MSET"] = func(args []string) (string, error) {
		index, err := argInt(args, 0)
		if err != nil {
			return "", err
		}
		value, err := argInt(args, 1)
		if err != nil {
			return "", err
		}
		if err := d.array.Set(index, value); err != nil {
			return "", err
		}
		return "OK", nil
	}
	d.handlers["MDEL"] = func(args []string) (string, error) {
		index, err := argInt(args, 0)
		if err != nil {
			return "", err
		}
		if err := d.array.Erase(index); err != nil {
			return "", err
		}
		return "OK", nil
	}
	d.handlers["MLEN"] = func(_ []string) (string, error) {
		return strconv.Itoa(d.array.Len()), nil
	}
}

func (d *Database) registerForwardList() {
	d.handlers["FPUSH"] = func(args []string) (string, error) {
		first, err := argInt(args, 0)
		if err != nil {
			return "", err
		}
		if len(args) < 2 {
			d.forwardList.PushFront(first)
			return "OK", nil
		}
		second, err := argInt(args, 1)
		if err != nil {
			return "", err
		}
		if err := d.forwardList.Insert(first, second); err != nil {
			return "", err
		}
		return "OK", nil
	}
	d.handlers["FPUSH_END"] = func(args []string) (string, error) {
		value, err := argInt(args, 0)
		if err != nil {
			return "", err
		}
		d.forwardList.PushBack(value)
		return "OK", nil
	}
	d.handlers["FPUSH_AFTER"] = func(args []string) (string, error) {
		index, err := argInt(args, 0)
		if err != nil {
			return "", err
		}
		value, err := argInt(args, 1)
		if err != nil {
			return "", err
		}
		if err := d.forwardList.InsertAfter(index, value); err != nil {
			return "", err
		}
		return "OK", nil
	}
	d.handlers["FGET"] = func(args []string) (string, error) {
		index, err := argInt(args, 0)
		if err != nil {
			return "", err
		}
		value, err := d.forwardList.Get(index)
		if err != nil {
			return "", err
		}
		return strconv.Itoa(value), nil
	}
	d.handlers["FDEL"] = func(args []string) (string, error) {
		index, err := argInt(args, 0)
		if err != nil {
			return "", err
		}
		if err := d.forwardList.EraseAt(index); err != nil {
			return "", err
		}
		return "OK", nil
	}
	d.handlers["FDEL_VALUE"] = func(args []string) (string, error) {
		value, err := argInt(args, 0)
		if err != nil {
			return "", err
		}
		if d.forwardList.EraseValue(value) {
			return "OK", nil
		}
		return "NOT_FOUND", nil
	}
	d.handlers["FSEARCH"] = func(args []string) (string, error) {
		value, err := argInt(args, 0)
		if err != nil {
			return "", err
		}
		index, found := d.forwardList.IndexOf(value)
		return indexOrMinusOne(index, found), nil
	}
}

func (d *Database) registerLinkedList() {
	d.handlers["LPUSH"] = func(args []string) (string, error) {
		first, err := argInt(args, 0)
		if err != nil {
			return "", err
		}
		if len(args) < 2 {
			d.linkedList.PushFront(first)
			return "OK", nil
		}
		second, err := argInt(args, 1)
		if err != nil {
			return "", err
		}
		if err := d.linkedList.Insert(first, second); err != nil {
			return "", err
		}
		return "OK", nil
	}
	d.handlers["LPUSH_END"] = func(args []string) (string, error) {
		value, err := argInt(args, 0)
		if err != nil {
			return "", err
		}
		d.linkedList.PushBack(value)
		return "OK", nil
	}
	d.handlers["LPUSH_AFTER"] = func(args []string) (string, error) {
		index, err := argInt(args, 0)
		if err != nil {
			return "", err
		}
		value, err := argInt(args, 1)
		if err != nil {
			return "", err
		}
		if err := d.linkedList.InsertAfter(index, value); err != nil {
			return "", err
		}
		return "OK", nil
	}
	d.handlers["LGET"] = func(args []string) (string, error) {
		index, err := argInt(args, 0)
		if err != nil {
			return "", err
		}
		value, err := d.linkedList.Get(index)
		if err != nil {
			return "", err
		}
		return strconv.Itoa(value), nil
	}
	d.handlers["LDEL"] = func(args []string) (string, error) {
		index, err := argInt(args, 0)
		if err != nil {
			return "", err
		}
		if err := d.linkedList.EraseAt(index); err != nil {
			return "", err
		}
		return "OK", nil
	}
	d.handlers["LDEL_VALUE"] = func(args []string) (string, error) {
		value, err := argInt(args, 0)
		if err != nil {
			return "", err
		}
		if d.linkedList.EraseValue(value) {
			return "OK", nil
		}
		return "NOT_FOUND", nil
	}
	d.handlers["LSEARCH"] = func(args []string) (string, error) {
		value, err := argInt(args, 0)
		if err != nil {
			return "", err
		}
		index, found := d.linkedList.IndexOf(value)
		return indexOrMinusOne(index, found), nil
	}
}

func (d *Database) registerStack() {
	d.handlers["SPUSH"] = func(args []string) (string, error) {
		value, err := argInt(args, 0)
		if err != nil {
			return "", err
		}
		d.stack.Push(value)
		return strconv.Itoa(value), nil
	}
	d.handlers["SPOP"] = func(_ []string) (string, error) {
		value, err := d.stack.Pop()
		if err != nil {
			return "", err
		}
		return strconv.Itoa(value), nil
	}
	d.handlers["SPEEK"] = func(_ []string) (string, error) {
		value, err := d.stack.Peek()
		if err != nil {
			return "", err
		}
		return strconv.Itoa(value), nil
	}
}

func (d *Database) registerQueue() {
	d.handlers["QPUSH"] = func(args []string) (string, error) {
		value, err := argInt(args, 0)
		if err != nil {
			return "", err
		}
		d.queue.Push(value)
		return strconv.Itoa(value), nil
	}
	d.handlers["QPOP"] = func(_ []string) (string, error) {
		value, err := d.queue.Pop()
		if err != nil {
			return "", err
		}
		return strconv.Itoa(value), nil
	}
	d.handlers["QPEEK"] = func(_ []string) (string, error) {
		value, err := d.queue.Peek()
		if err != nil {
			return "", err
		}
		return strconv.Itoa(value), nil
	}
}

func (d *Database) registerTree() {
	d.handlers["TINSERT"] = func(args []string) (string, error) {
		value, err := argInt(args, 0)
		if err != nil {
			return "", err
		}
		d.tree.Insert(value)
		return strconv.Itoa(value), nil
	}
	d.handlers["TGET"] = func(args []string) (string, error) {
		index, err := argInt(args, 0)
		if err != nil {
			return "", err
		}
		value, err := d.tree.Get(index)
		if err != nil {
			return "", err
		}
		return strconv.Itoa(value), nil
	}
	d.handlers["TSEARCH"] = func(args []string) (string, error) {
		value, err := argInt(args, 0)
		if err != nil {
			return "", err
		}
		index, found := d.tree.IndexOf(value)
		return indexOrMinusOne(index, found), nil
	}
	d.handlers["TDEL"] = func(args []string) (string, error) {
		index, err := argInt(args, 0)
		if err != nil {
			return "", err
		}
		if err := d.tree.Erase(index); err != nil {
			return "", err
		}
		return "OK", nil
	}
	d.handlers["TFULL"] = func(_ []string) (string, error) {
		return boolText(d.tree.IsFull()), nil
	}
	d.handlers["THEIGHT"] = func(_ []string) (string, error) {
		return strconv.Itoa(d.tree.Height()), nil
	}
	d.handlers["TORDER"] = func(args []string) (string, error) {
		kind, err := argKey(args, 0)
		if err != nil {
			return "", err
		}
		switch kind {
		case "pre":
			return joinValues(d.tree.Preorder()), nil
		case "in":
			return joinValues(d.tree.Inorder()), nil
		case "post":
			return joinValues(d.tree.Postorder()), nil
		case "level":
			return joinValues(d.tree.LevelOrder()), nil
		default:
			return "", fmt.Errorf("%w: unknown traversal %s", ErrFormat, kind)
		}
	}
}

func (d *Database) registerHashTable() {
	d.handlers["HPUT"] = func(args []string) (string, error) {
		key, err := argKey(args, 0)
		if err != nil {
			return "", err
		}
		value, err := argInt(args, 1)
		if err != nil {
			return "", err
		}
		d.hashTable.Put(key, value)
		return "OK", nil
	}
	d.handlers["HGET"] = func(args []string) (string, error) {
		key, err := argKey(args, 0)
		if err != nil {
			return "", err
		}
		value, found := d.hashTable.Get(key)
		if !found {
			return "NOT_FOUND", nil
		}
		return strconv.Itoa(value), nil
	}
	d.handlers["HDEL"] = func(args []string) (string, error) {
		key, err := argKey(args, 0)
		if err != nil {
			return "", err
		}
		return boolText(d.hashTable.Erase(key)), nil
	}
	d.handlers["HHAS"] = func(args []string) (string, error) {
		key, err := argKey(args, 0)
		if err != nil {
			return "", err
		}
		return boolText(d.hashTable.Contains(key)), nil
	}
	d.handlers["HSIZE"] = func(_ []string) (string, error) {
		return strconv.Itoa(d.hashTable.Len()), nil
	}
}

func (d *Database) registerCommon() {
	d.handlers["PRINT"] = func(args []string) (string, error) {
		what, err := argKey(args, 0)
		if err != nil {
			return "", err
		}
		switch what {
		case "M":
			return d.array.String(), nil
		case "F":
			return d.forwardList.String(), nil
		case "L":
			return d.linkedList.String(), nil
		case "LR":
			return d.linkedList.ReversedString(), nil
		case "S":
			return d.stack.String(), nil
		case "Q":
			return d.queue.String(), nil
		case "T":
			return d.tree.String(), nil
		case "H":
			return d.hashTable.String(), nil
		default:
			return "", fmt.Errorf("%w: unknown container %s", ErrFormat, what)
		}
	}
	d.handlers["STATS"] = func(_ []string) (string, error) {
		lines := make([]string, 0, 7)
		for _, container := range d.Containers() {
			lines = append(lines, fmt.Sprintf("%s: %d", container.Kind(), container.Len()))
		}
		return strings.Join(lines, "\n"), nil
	}
}
