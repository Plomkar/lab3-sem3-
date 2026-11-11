package ds

import (
	"encoding/binary"
	"fmt"
	"math"
	"os"
	"path/filepath"
)

const (
	binaryMagic   = "DSDB"
	binaryVersion = 1
	maxCount      = 1 << 28
	maxStringLen  = 1 << 24
)

type binaryReader struct {
	data []byte
	pos  int
}

func (r *binaryReader) next(size int) ([]byte, error) {
	if size < 0 || size > len(r.data)-r.pos {
		return nil, fmt.Errorf("%w: unexpected end of binary data", ErrFormat)
	}
	chunk := r.data[r.pos : r.pos+size]
	r.pos += size
	return chunk, nil
}

func (r *binaryReader) expectTag(tag string) error {
	chunk, err := r.next(len(tag))
	if err != nil {
		return err
	}
	if string(chunk) != tag {
		return fmt.Errorf("%w: unexpected block tag, expected %s", ErrFormat, tag)
	}
	return nil
}

func (r *binaryReader) u8() (byte, error) {
	chunk, err := r.next(1)
	if err != nil {
		return 0, err
	}
	return chunk[0], nil
}

func (r *binaryReader) u64() (uint64, error) {
	chunk, err := r.next(8)
	if err != nil {
		return 0, err
	}
	return binary.LittleEndian.Uint64(chunk), nil
}

func (r *binaryReader) count() (int, error) {
	value, err := r.u64()
	if err != nil {
		return 0, err
	}
	if value > maxCount {
		return 0, fmt.Errorf("%w: element count is too large: %d", ErrFormat, value)
	}
	return int(value), nil
}

func (r *binaryReader) int32() (int, error) {
	chunk, err := r.next(4)
	if err != nil {
		return 0, err
	}
	return int(int32(binary.LittleEndian.Uint32(chunk))), nil
}

func (r *binaryReader) str() (string, error) {
	length, err := r.u64()
	if err != nil {
		return "", err
	}
	if length > maxStringLen {
		return "", fmt.Errorf("%w: string is too long: %d", ErrFormat, length)
	}
	chunk, err := r.next(int(length))
	if err != nil {
		return "", err
	}
	return string(chunk), nil
}

func appendIntBlock(buf []byte, tag string, values []int) []byte {
	buf = append(buf, tag...)
	buf = binary.LittleEndian.AppendUint64(buf, uint64(len(values)))
	for _, value := range values {
		buf = binary.LittleEndian.AppendUint32(buf, uint32(int32(value)))
	}
	return buf
}

func readIntBlock(r *binaryReader, tag string) ([]int, error) {
	if err := r.expectTag(tag); err != nil {
		return nil, err
	}
	count, err := r.count()
	if err != nil {
		return nil, err
	}
	if count > (len(r.data)-r.pos)/4 {
		return nil, fmt.Errorf("%w: unexpected end of binary data", ErrFormat)
	}
	values := make([]int, count)
	for i := range values {
		values[i] = int(int32(binary.LittleEndian.Uint32(r.data[r.pos:])))
		r.pos += 4
	}
	return values, nil
}

func fitsInt32(value int) bool {
	return value >= math.MinInt32 && value <= math.MaxInt32
}

func (d *Database) validateInt32() error {
	groups := [][]int{
		d.array.ToSlice(),
		d.forwardList.ToSlice(),
		d.linkedList.ToSlice(),
		d.stack.ToSlice(),
		d.queue.ToSlice(),
		d.tree.LevelOrder(),
	}
	for _, values := range groups {
		for _, value := range values {
			if !fitsInt32(value) {
				return fmt.Errorf("%w: value %d does not fit into 32 bits", ErrFormat, value)
			}
		}
	}
	for _, key := range d.hashTable.Keys() {
		value, _ := d.hashTable.Get(key)
		if !fitsInt32(value) {
			return fmt.Errorf("%w: value %d does not fit into 32 bits", ErrFormat, value)
		}
	}
	return nil
}

func (d *Database) appendHashBlock(buf []byte) []byte {
	keys := d.hashTable.Keys()
	buf = append(buf, "HASH"...)
	buf = append(buf, byte(d.hashTable.Strategy()))
	buf = binary.LittleEndian.AppendUint64(buf, uint64(len(keys)))
	for _, key := range keys {
		value, _ := d.hashTable.Get(key)
		buf = binary.LittleEndian.AppendUint64(buf, uint64(len(key)))
		buf = append(buf, key...)
		buf = binary.LittleEndian.AppendUint32(buf, uint32(int32(value)))
	}
	return buf
}

func readHashBlock(r *binaryReader) (*HashTable[int], error) {
	if err := r.expectTag("HASH"); err != nil {
		return nil, err
	}
	raw, err := r.u8()
	if err != nil {
		return nil, err
	}
	if raw > byte(DoubleHashing) {
		return nil, fmt.Errorf("%w: unknown probe strategy %d", ErrFormat, raw)
	}
	count, err := r.count()
	if err != nil {
		return nil, err
	}
	table := NewHashTable[int](ProbeStrategy(raw), 11)
	for i := 0; i < count; i++ {
		key, keyErr := r.str()
		if keyErr != nil {
			return nil, keyErr
		}
		value, valueErr := r.int32()
		if valueErr != nil {
			return nil, valueErr
		}
		table.Put(key, value)
	}
	return table, nil
}

func (d *Database) MarshalBinary() ([]byte, error) {
	if err := d.validateInt32(); err != nil {
		return nil, err
	}
	buf := make([]byte, 0, 256)
	buf = append(buf, binaryMagic...)
	buf = append(buf, binaryVersion)
	buf = appendIntBlock(buf, "ARRY", d.array.ToSlice())
	buf = appendIntBlock(buf, "FLST", d.forwardList.ToSlice())
	buf = appendIntBlock(buf, "LLST", d.linkedList.ToSlice())
	buf = append(buf, "STCK"...)
	buf = appendIntBlock(buf, "FLST", d.stack.ToSlice())
	buf = append(buf, "QUEU"...)
	buf = appendIntBlock(buf, "LLST", d.queue.ToSlice())
	buf = appendIntBlock(buf, "CBTR", d.tree.LevelOrder())
	return d.appendHashBlock(buf), nil
}

func (d *Database) UnmarshalBinary(data []byte) error {
	r := &binaryReader{data: data}
	if err := r.expectTag(binaryMagic); err != nil {
		return err
	}
	version, err := r.u8()
	if err != nil {
		return err
	}
	if version != binaryVersion {
		return fmt.Errorf("%w: unsupported version %d", ErrFormat, version)
	}
	arrayItems, err := readIntBlock(r, "ARRY")
	if err != nil {
		return err
	}
	forwardItems, err := readIntBlock(r, "FLST")
	if err != nil {
		return err
	}
	linkedItems, err := readIntBlock(r, "LLST")
	if err != nil {
		return err
	}
	if err = r.expectTag("STCK"); err != nil {
		return err
	}
	stackItems, err := readIntBlock(r, "FLST")
	if err != nil {
		return err
	}
	if err = r.expectTag("QUEU"); err != nil {
		return err
	}
	queueItems, err := readIntBlock(r, "LLST")
	if err != nil {
		return err
	}
	treeItems, err := readIntBlock(r, "CBTR")
	if err != nil {
		return err
	}
	table, err := readHashBlock(r)
	if err != nil {
		return err
	}
	d.array = NewArray(arrayItems...)
	d.forwardList = NewForwardList(forwardItems...)
	d.linkedList = NewLinkedList(linkedItems...)
	d.stack = &Stack[int]{items: NewForwardList(stackItems...)}
	d.queue = &Queue[int]{items: NewLinkedList(queueItems...)}
	d.tree = NewCompleteBinaryTree(treeItems...)
	d.hashTable = table
	return nil
}

func (d *Database) SaveBinaryFile(path string) error {
	data, err := d.MarshalBinary()
	if err != nil {
		return err
	}
	return os.WriteFile(path, data, 0o600)
}

func (d *Database) LoadBinaryFile(path string) error {
	data, err := os.ReadFile(filepath.Clean(path))
	if err != nil {
		return err
	}
	return d.UnmarshalBinary(data)
}
