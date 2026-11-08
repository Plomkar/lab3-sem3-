#pragma once

#include <cstddef>
#include <initializer_list>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "ds/container.hpp"
#include "ds/errors.hpp"
#include "ds/format.hpp"

namespace ds {

template <typename T>
class ForwardList : public Container {
    static_assert(std::is_trivially_copyable_v<T>, "ForwardList requires a trivially copyable element type");

public:
    ForwardList() = default;

    ForwardList(std::initializer_list<T> items) {
        for (const T& item : items) {
            pushBack(item);
        }
    }

    ForwardList(const ForwardList& other) : Container(other) {
        for (const Node* cur = other.head_; cur != nullptr; cur = cur->next) {
            pushBack(cur->value);
        }
    }

    ForwardList(ForwardList&& other) noexcept
        : Container(std::move(other)), head_(other.head_), tail_(other.tail_), size_(other.size_) {
        other.head_ = nullptr;
        other.tail_ = nullptr;
        other.size_ = 0;
    }

    ForwardList& operator=(const ForwardList& other) {
        if (this != &other) {
            ForwardList copy(other);
            swap(copy);
        }
        return *this;
    }

    ForwardList& operator=(ForwardList&& other) noexcept {
        if (this != &other) {
            release();
            swap(other);
        }
        return *this;
    }

    ~ForwardList() override { release(); }

    void swap(ForwardList& other) noexcept {
        std::swap(head_, other.head_);
        std::swap(tail_, other.tail_);
        std::swap(size_, other.size_);
    }

    void pushFront(const T& value) {
        Node* node = new Node{value, head_};
        head_ = node;
        if (tail_ == nullptr) {
            tail_ = node;
        }
        ++size_;
    }

    void pushBack(const T& value) {
        Node* node = new Node{value, nullptr};
        if (tail_ != nullptr) {
            tail_->next = node;
        } else {
            head_ = node;
        }
        tail_ = node;
        ++size_;
    }

    void insert(std::size_t index, const T& value) {
        if (index > size_) {
            throw IndexError("insert index out of range: " + std::to_string(index));
        }
        if (index == 0) {
            pushFront(value);
        } else if (index == size_) {
            pushBack(value);
        } else {
            Node* prev = nodeAt(index - 1);
            prev->next = new Node{value, prev->next};
            ++size_;
        }
    }

    void insertAfter(std::size_t index, const T& value) {
        checkIndex(index);
        Node* cur = nodeAt(index);
        Node* node = new Node{value, cur->next};
        cur->next = node;
        if (cur == tail_) {
            tail_ = node;
        }
        ++size_;
    }

    T popFront() {
        if (head_ == nullptr) {
            throw EmptyError("forward list is empty");
        }
        Node* removed = head_;
        const T value = removed->value;
        head_ = removed->next;
        if (head_ == nullptr) {
            tail_ = nullptr;
        }
        delete removed;
        --size_;
        return value;
    }

    const T& front() const {
        if (head_ == nullptr) {
            throw EmptyError("forward list is empty");
        }
        return head_->value;
    }

    const T& back() const {
        if (tail_ == nullptr) {
            throw EmptyError("forward list is empty");
        }
        return tail_->value;
    }

    void eraseAt(std::size_t index) {
        checkIndex(index);
        if (index == 0) {
            popFront();
            return;
        }
        Node* prev = nodeAt(index - 1);
        Node* removed = prev->next;
        prev->next = removed->next;
        if (removed == tail_) {
            tail_ = prev;
        }
        delete removed;
        --size_;
    }

    bool eraseValue(const T& value) {
        Node* prev = nullptr;
        for (Node* cur = head_; cur != nullptr; prev = cur, cur = cur->next) {
            if (cur->value == value) {
                if (prev != nullptr) {
                    prev->next = cur->next;
                } else {
                    head_ = cur->next;
                }
                if (cur == tail_) {
                    tail_ = prev;
                }
                delete cur;
                --size_;
                return true;
            }
        }
        return false;
    }

    const T& get(std::size_t index) const {
        checkIndex(index);
        return nodeAt(index)->value;
    }

    std::optional<std::size_t> indexOf(const T& value) const {
        std::size_t index = 0;
        for (const Node* cur = head_; cur != nullptr; cur = cur->next, ++index) {
            if (cur->value == value) {
                return index;
            }
        }
        return std::nullopt;
    }

    std::vector<T> toVector() const {
        std::vector<T> items;
        items.reserve(size_);
        for (const Node* cur = head_; cur != nullptr; cur = cur->next) {
            items.push_back(cur->value);
        }
        return items;
    }

    std::size_t size() const noexcept override { return size_; }

    void clear() noexcept override { release(); }

    std::string toString() const override {
        const std::vector<T> items = toVector();
        return joinValues(items.begin(), items.end());
    }

    std::string kind() const override { return "ForwardList"; }

    void save(BinaryWriter& writer) const override {
        writer.writeTag("FLST");
        writer.writeU64(size_);
        for (const Node* cur = head_; cur != nullptr; cur = cur->next) {
            writer.writePod(cur->value);
        }
    }

    void load(BinaryReader& reader) override {
        reader.expectTag("FLST");
        const std::uint64_t count = reader.readCount();
        ForwardList fresh;
        for (std::uint64_t i = 0; i < count; ++i) {
            fresh.pushBack(reader.readPod<T>());
        }
        swap(fresh);
    }

private:
    struct Node {
        T value;
        Node* next;
    };

    void checkIndex(std::size_t index) const {
        if (index >= size_) {
            throw IndexError("index out of range: " + std::to_string(index));
        }
    }

    Node* nodeAt(std::size_t index) const {
        Node* cur = head_;
        for (std::size_t i = 0; i < index; ++i) {
            cur = cur->next;
        }
        return cur;
    }

    void release() noexcept {
        while (head_ != nullptr) {
            Node* removed = head_;
            head_ = head_->next;
            delete removed;
        }
        tail_ = nullptr;
        size_ = 0;
    }

    Node* head_ = nullptr;
    Node* tail_ = nullptr;
    std::size_t size_ = 0;
};

}  // namespace ds
