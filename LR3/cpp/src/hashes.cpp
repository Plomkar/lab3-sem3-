#include "ds/hashes.hpp"

#include <numeric>

namespace ds {

std::uint64_t polynomialHash(const std::string& key) noexcept {
    constexpr std::uint64_t kBase = 131;
    return std::accumulate(key.begin(), key.end(), std::uint64_t{0}, [](std::uint64_t hash, char symbol) {
        return hash * kBase + static_cast<unsigned char>(symbol);
    });
}

std::uint64_t fnv1aHash(const std::string& key) noexcept {
    constexpr std::uint64_t kOffset = 14695981039346656037ULL;
    constexpr std::uint64_t kPrime = 1099511628211ULL;
    std::uint64_t hash = kOffset;
    for (const char symbol : key) {
        hash ^= static_cast<unsigned char>(symbol);
        hash *= kPrime;
    }
    return hash;
}

bool isPrime(std::size_t value) noexcept {
    if (value < 2) {
        return false;
    }
    for (std::size_t divisor = 2; divisor * divisor <= value; ++divisor) {
        if (value % divisor == 0) {
            return false;
        }
    }
    return true;
}

std::size_t nextPrime(std::size_t value) noexcept {
    while (!isPrime(value)) {
        ++value;
    }
    return value;
}

}  // namespace ds
