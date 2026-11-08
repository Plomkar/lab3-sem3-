#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "ds/container.hpp"
#include "ds/errors.hpp"
#include "ds/format.hpp"
#include "ds/hashes.hpp"

namespace ds {

enum class ProbeStrategy : std::uint8_t { Linear = 0, Double = 1 };

template <typename V>
class HashTable : public Container {
    static_assert(std::is_trivially_copyable_v<V>, "HashTable requires a trivially copyable value type");

public:
    explicit HashTable(ProbeStrategy strategy = ProbeStrategy::Double, std::size_t initialCapacity = 11)
        : slots_(nextPrime(std::max<std::size_t>(initialCapacity, 3))), strategy_(strategy) {}


    void put(const std::string& key, const V& value) {
        const std::size_t found = findIndex(key);
        if (found != kNotFound) {
            slots_[found].value = value;
            return;
        }
        if ((filled_ + 1) * 10 > slots_.size() * 7) {
            const std::size_t target = (used_ + 1) * 2 > slots_.size() ? slots_.size() * 2 : slots_.size();
            rehash(target);
        }
        for (std::size_t attempt = 0;; ++attempt) {
            Slot& slot = slots_[probe(key, attempt)];
            if (slot.state != State::Used) {
                if (slot.state == State::Empty) {
                    ++filled_;
                }
                slot.key = key;
                slot.value = value;
                slot.state = State::Used;
                ++used_;
                return;
            }
        }
    }

    std::optional<V> get(const std::string& key) const {
        const std::size_t found = findIndex(key);
        if (found == kNotFound) {
            return std::nullopt;
        }
        return slots_[found].value;
    }

    bool contains(const std::string& key) const { return findIndex(key) != kNotFound; }

    bool erase(const std::string& key) {
        const std::size_t found = findIndex(key);
        if (found == kNotFound) {
            return false;
        }
        slots_[found].state = State::Deleted;
        --used_;
        return true;
    }

    std::size_t capacity() const noexcept { return slots_.size(); }

    ProbeStrategy strategy() const noexcept { return strategy_; }

    double loadFactor() const noexcept {
        return static_cast<double>(filled_) / static_cast<double>(slots_.size());
    }

    std::vector<std::string> keys() const {
        std::vector<std::string> result;
        result.reserve(used_);
        for (const Slot& slot : slots_) {
            if (slot.state == State::Used) {
                result.push_back(slot.key);
            }
        }
        std::sort(result.begin(), result.end());
        return result;
    }

    std::size_t size() const noexcept override { return used_; }

    void clear() noexcept override {
        for (Slot& slot : slots_) {
            slot.state = State::Empty;
        }
        used_ = 0;
        filled_ = 0;
    }

    std::string toString() const override {
        std::string result;
        for (const std::string& key : keys()) {
            if (!result.empty()) {
                result += ' ';
            }
            result += key + "=" + formatValue(slots_[findIndex(key)].value);
        }
        return result;
    }

    std::string kind() const override { return "HashTable"; }

    void save(BinaryWriter& writer) const override {
        writer.writeTag("HASH");
        writer.writePod(static_cast<std::uint8_t>(strategy_));
        writer.writeU64(used_);
        for (const Slot& slot : slots_) {
            if (slot.state == State::Used) {
                writer.writeString(slot.key);
                writer.writePod(slot.value);
            }
        }
    }

    void load(BinaryReader& reader) override {
        reader.expectTag("HASH");
        const auto raw = reader.readPod<std::uint8_t>();
        if (raw > static_cast<std::uint8_t>(ProbeStrategy::Double)) {
            throw FormatError("unknown probe strategy: " + std::to_string(raw));
        }
        const std::uint64_t count = reader.readCount();
        HashTable fresh(static_cast<ProbeStrategy>(raw));
        for (std::uint64_t i = 0; i < count; ++i) {
            const std::string key = reader.readString();
            fresh.put(key, reader.readPod<V>());
        }
        *this = std::move(fresh);
    }

private:
    enum class State : std::uint8_t { Empty, Used, Deleted };

    struct Slot {
        std::string key;
        V value{};
        State state = State::Empty;
    };

    static constexpr std::size_t kNotFound = static_cast<std::size_t>(-1);

    std::size_t probe(const std::string& key, std::size_t attempt) const {
        const std::size_t slotCount = slots_.size();
        const auto start = static_cast<std::size_t>(polynomialHash(key) % slotCount);
        std::size_t step = 1;
        if (strategy_ == ProbeStrategy::Double) {
            step = 1 + static_cast<std::size_t>(fnv1aHash(key) % (slotCount - 1));
        }
        return (start + attempt * step) % slotCount;
    }

    std::size_t findIndex(const std::string& key) const {
        for (std::size_t attempt = 0; attempt < slots_.size(); ++attempt) {
            const std::size_t index = probe(key, attempt);
            const Slot& slot = slots_[index];
            if (slot.state == State::Empty) {
                return kNotFound;
            }
            if (slot.state == State::Used && slot.key == key) {
                return index;
            }
        }
        return kNotFound;
    }

    void rehash(std::size_t newCapacity) {
        std::vector<Slot> old(nextPrime(newCapacity));
        old.swap(slots_);
        used_ = 0;
        filled_ = 0;
        for (const Slot& slot : old) {
            if (slot.state == State::Used) {
                put(slot.key, slot.value);
            }
        }
    }

    std::vector<Slot> slots_;
    std::size_t used_ = 0;
    std::size_t filled_ = 0;
    ProbeStrategy strategy_;
};

}  // namespace ds
