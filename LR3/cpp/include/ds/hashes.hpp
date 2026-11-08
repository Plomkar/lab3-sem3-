#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace ds {

std::uint64_t polynomialHash(const std::string& key) noexcept;
std::uint64_t fnv1aHash(const std::string& key) noexcept;
bool isPrime(std::size_t value) noexcept;
std::size_t nextPrime(std::size_t value) noexcept;

}  // namespace ds
