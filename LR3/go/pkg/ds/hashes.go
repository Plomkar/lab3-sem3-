package ds

func polynomialHash(key string) uint64 {
	const base uint64 = 131
	var hash uint64
	for i := 0; i < len(key); i++ {
		hash = hash*base + uint64(key[i])
	}
	return hash
}

func fnv1aHash(key string) uint64 {
	const offset uint64 = 14695981039346656037
	const prime uint64 = 1099511628211
	hash := offset
	for i := 0; i < len(key); i++ {
		hash ^= uint64(key[i])
		hash *= prime
	}
	return hash
}

func isPrime(value int) bool {
	if value < 2 {
		return false
	}
	for divisor := 2; divisor*divisor <= value; divisor++ {
		if value%divisor == 0 {
			return false
		}
	}
	return true
}

func nextPrime(value int) int {
	for !isPrime(value) {
		value++
	}
	return value
}
