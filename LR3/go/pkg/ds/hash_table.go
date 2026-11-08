package ds

import (
	"encoding/json"
	"fmt"
	"sort"
)

type ProbeStrategy int

const (
	LinearProbing ProbeStrategy = iota
	DoubleHashing
)

func (s ProbeStrategy) String() string {
	if s == LinearProbing {
		return "linear"
	}
	return "double"
}

func parseStrategy(name string) (ProbeStrategy, error) {
	switch name {
	case "linear":
		return LinearProbing, nil
	case "double":
		return DoubleHashing, nil
	default:
		return LinearProbing, fmt.Errorf("%w: unknown probe strategy %q", ErrFormat, name)
	}
}

type slotState uint8

const (
	slotEmpty slotState = iota
	slotUsed
	slotDeleted
)

type hashSlot[V any] struct {
	key   string
	value V
	state slotState
}

type HashTable[V any] struct {
	slots    []hashSlot[V]
	used     int
	filled   int
	strategy ProbeStrategy
}

type hashTableJSON[V any] struct {
	Strategy string       `json:"strategy"`
	Entries  map[string]V `json:"entries"`
}

func NewHashTable[V any](strategy ProbeStrategy, initialCapacity int) *HashTable[V] {
	if initialCapacity < 3 {
		initialCapacity = 3
	}
	return &HashTable[V]{
		slots:    make([]hashSlot[V], nextPrime(initialCapacity)),
		strategy: strategy,
	}
}

func (h *HashTable[V]) Len() int { return h.used }

func (h *HashTable[V]) Kind() string { return "HashTable" }

func (h *HashTable[V]) Capacity() int { return len(h.slots) }

func (h *HashTable[V]) Strategy() ProbeStrategy { return h.strategy }

func (h *HashTable[V]) LoadFactor() float64 {
	if len(h.slots) == 0 {
		return 0
	}
	return float64(h.filled) / float64(len(h.slots))
}

func (h *HashTable[V]) Clear() {
	for i := range h.slots {
		h.slots[i] = hashSlot[V]{}
	}
	h.used = 0
	h.filled = 0
}

func (h *HashTable[V]) Put(key string, value V) {
	if len(h.slots) == 0 {
		h.slots = make([]hashSlot[V], nextPrime(11))
	}
	if found := h.findIndex(key); found >= 0 {
		h.slots[found].value = value
		return
	}
	if (h.filled+1)*10 > len(h.slots)*7 {
		target := len(h.slots)
		if (h.used+1)*2 > target {
			target = len(h.slots) * 2
		}
		h.rehash(target)
	}
	for attempt := 0; ; attempt++ {
		slot := &h.slots[h.probe(key, attempt)]
		if slot.state == slotUsed {
			continue
		}
		if slot.state == slotEmpty {
			h.filled++
		}
		slot.key = key
		slot.value = value
		slot.state = slotUsed
		h.used++
		return
	}
}

func (h *HashTable[V]) Get(key string) (V, bool) {
	found := h.findIndex(key)
	if found < 0 {
		var zero V
		return zero, false
	}
	return h.slots[found].value, true
}

func (h *HashTable[V]) Contains(key string) bool {
	return h.findIndex(key) >= 0
}

func (h *HashTable[V]) Erase(key string) bool {
	found := h.findIndex(key)
	if found < 0 {
		return false
	}
	h.slots[found].state = slotDeleted
	h.used--
	return true
}

func (h *HashTable[V]) Keys() []string {
	keys := make([]string, 0, h.used)
	for i := range h.slots {
		if h.slots[i].state == slotUsed {
			keys = append(keys, h.slots[i].key)
		}
	}
	sort.Strings(keys)
	return keys
}

func (h *HashTable[V]) String() string {
	result := ""
	for i, key := range h.Keys() {
		if i > 0 {
			result += " "
		}
		value, _ := h.Get(key)
		result += fmt.Sprintf("%s=%v", key, value)
	}
	return result
}

func (h *HashTable[V]) MarshalJSON() ([]byte, error) {
	entries := make(map[string]V, h.used)
	for i := range h.slots {
		if h.slots[i].state == slotUsed {
			entries[h.slots[i].key] = h.slots[i].value
		}
	}
	return json.Marshal(hashTableJSON[V]{Strategy: h.strategy.String(), Entries: entries})
}

func (h *HashTable[V]) UnmarshalJSON(data []byte) error {
	var payload hashTableJSON[V]
	if err := json.Unmarshal(data, &payload); err != nil {
		return fmt.Errorf("%w: %w", ErrFormat, err)
	}
	strategy, err := parseStrategy(payload.Strategy)
	if err != nil {
		return err
	}
	fresh := NewHashTable[V](strategy, 11)
	for key, value := range payload.Entries {
		fresh.Put(key, value)
	}
	*h = *fresh
	return nil
}

func (h *HashTable[V]) probe(key string, attempt int) int {
	capacity := len(h.slots)
	start := int(polynomialHash(key) % uint64(capacity))
	step := 1
	if h.strategy == DoubleHashing {
		step = 1 + int(fnv1aHash(key)%uint64(capacity-1))
	}
	return (start + attempt*step) % capacity
}

func (h *HashTable[V]) findIndex(key string) int {
	for attempt := 0; attempt < len(h.slots); attempt++ {
		index := h.probe(key, attempt)
		slot := &h.slots[index]
		if slot.state == slotEmpty {
			return -1
		}
		if slot.state == slotUsed && slot.key == key {
			return index
		}
	}
	return -1
}

func (h *HashTable[V]) rehash(newCapacity int) {
	old := h.slots
	h.slots = make([]hashSlot[V], nextPrime(newCapacity))
	h.used = 0
	h.filled = 0
	for i := range old {
		if old[i].state == slotUsed {
			h.Put(old[i].key, old[i].value)
		}
	}
}
