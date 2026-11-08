#pragma once

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include "ds/container.hpp"
#include "ds/errors.hpp"
#include "ds/format.hpp"

namespace ds {

template <typename T>
class Array : public Container {
    static_assert(std::is_trivially_copyable_v<T>, "Array requires a trivially copyable element type");

public:
    Array() = default;

    Array(std::initializer_list<T> items) {
        reserve(items.size());
        for (const T& item : items) {
            pushBack(item);
        }
    }

    Array(const Array& other)
        : Container(other), data_(std::make_unique<T[]>(other.size_)), size_(other.size_), capacity_(other.size_) {
        std::copy(other.begin(), other.end(), data_.get());
    }

    Array(Array&& other) noexcept
        : Container(std::move(other)), data_(std::move(other.data_)), size_(other.size_), capacity_(other.capacity_) {
        other.size_ = 0;
        other.capacity_ = 0;
    }

    Array& operator=(const Array& other) {
        if (this != &other) {
            Array copy(other);
            swap(copy);
        }
        return *this;
    }

    Array& operator=(Array&& other) noexcept {
        if (this != &other) {
            data_ = std::move(other.data_);
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    ~Array() override = default;

    void swap(Array& other) noexcept {
        data_.swap(other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void pushBack(const T& value) {
        grow(size_ + 1);
        data_[size_] = value;
        ++size_;
    }

    void insert(std::size_t index, const T& value) {
        if (index > size_) {
            throw IndexError("insert index out of range: " + std::to_string(index));
        }
        grow(size_ + 1);
        for (std::size_t i = size_; i > index; --i) {
            data_[i] = data_[i - 1];
        }
        data_[index] = value;
        ++size_;
    }

    const T& get(std::size_t index) const {
        checkIndex(index);
        return data_[index];
    }

    void set(std::size_t index, const T& value) {
        checkIndex(index);
        data_[index] = value;
    }

    void erase(std::size_t index) {
        checkIndex(index);
        for (std::size_t i = index + 1; i < size_; ++i) {
            data_[i - 1] = data_[i];
        }
        --size_;
    }

    void reserve(std::size_t newCapacity) {
        if (newCapacity <= capacity_) {
            return;
        }
        auto fresh = std::make_unique<T[]>(newCapacity);
        std::copy(begin(), end(), fresh.get());
        data_ = std::move(fresh);
        capacity_ = newCapacity;
    }

    std::size_t capacity() const noexcept { return capacity_; }

    T& operator[](std::size_t index) {
        checkIndex(index);
        return data_[index];
    }

    const T& operator[](std::size_t index) const {
        checkIndex(index);
        return data_[index];
    }

    T* begin() noexcept { return data_.get(); }
    T* end() noexcept { return begin() + size_; }
    const T* begin() const noexcept { return data_.get(); }
    const T* end() const noexcept { return begin() + size_; }

    bool operator==(const Array& other) const {
        return size_ == other.size_ && std::equal(begin(), end(), other.begin());
    }

    bool operator!=(const Array& other) const { return !(*this == other); }

    std::size_t size() const noexcept override { return size_; }

    void clear() noexcept override { size_ = 0; }

    std::string toString() const override { return joinValues(begin(), end()); }

    std::string kind() const override { return "Array"; }

    void save(BinaryWriter& writer) const override {
        writer.writeTag("ARRY");
        writer.writeU64(size_);
        for (std::size_t i = 0; i < size_; ++i) {
            writer.writePod(data_[i]);
        }
    }

    void load(BinaryReader& reader) override {
        reader.expectTag("ARRY");
        const auto count = static_cast<std::size_t>(reader.readCount());
        Array fresh;
        fresh.reserve(count);
        for (std::size_t i = 0; i < count; ++i) {
            fresh.pushBack(reader.readPod<T>());
        }
        swap(fresh);
    }

private:
    void checkIndex(std::size_t index) const {
        if (index >= size_) {
            throw IndexError("index out of range: " + std::to_string(index));
        }
    }

    void grow(std::size_t minCapacity) {
        if (minCapacity <= capacity_) {
            return;
        }
        std::size_t next = capacity_ == 0 ? 4 : capacity_ * 2;
        if (next < minCapacity) {
            next = minCapacity;
        }
        reserve(next);
    }

    std::unique_ptr<T[]> data_;
    std::size_t size_ = 0;
    std::size_t capacity_ = 0;
};

}  // namespace ds
