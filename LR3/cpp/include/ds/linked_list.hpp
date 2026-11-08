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
class LinkedList : public Container {
    static_assert(std::is_trivially_copyable_v<T>, "LinkedList requires a trivially copyable element type");

public:
    LinkedList() = default;

    LinkedList(std::initializer_list<T> items) {
        for (const T& item : items) {
            pushBack(item);
        }
    }

    LinkedList(const LinkedList& other) : Container(other) {
        for (const Node* cur = other.head_; cur != nullptr; cur = cur->next) {
            pushBack(cur->value);
        }
    }

    LinkedList(LinkedList&& other) noexcept
        : Container(std::move(other)), head_(other.head_), tail_(other.tail_), size_(other.size_) {
        other.head_ = nullptr;
        other.tail_ = nullptr;
        other.size_ = 0;
    }

    LinkedList& operator=(const LinkedList& other) {
        if (this != &other) {
            LinkedList copy(other);
            swap(copy);
        }
        return *this;
    }

    LinkedList& operator=(LinkedList&& other) noexcept {
        if (this != &other) {
            release();
            swap(other);
        }
        return *this;
    }

    ~LinkedList() override { release(); }

    void swap(LinkedList& other) noexcept {
        std::swap(head_, other.head_);
        std::swap(tail_, other.tail_);
        std::swap(size_, other.size_);
    }

    void pushFront(const T& value) {
        Node* node = new Node{value, nullptr, head_};
        if (head_ != nullptr) {
            head_->prev = node;
        } else {
            tail_ = node;
        }
        head_ = node;
        ++size_;
    }

    void pushBack(const T& value) {
        Node* node = new Node{value, tail_, nullptr};
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
            Node* next = nodeAt(index);
            Node* prev = next->prev;
            Node* node = new Node{value, prev, next};
            prev->next = node;
            next->prev = node;
            ++size_;
        }
    }

    void insertAfter(std::size_t index, const T& value) {
        checkIndex(index);
        Node* cur = nodeAt(index);
        if (cur == tail_) {
            pushBack(value);
            return;
        }
        Node* next = cur->next;
        Node* node = new Node{value, cur, next};
        cur->next = node;
        next->prev = node;
        ++size_;
    }

    T popFront() {
        if (head_ == nullptr) {
            throw EmptyError("linked list is empty");
        }
        const T value = head_->value;
        unlink(head_);
        return value;
    }

    T popBack() {
        if (tail_ == nullptr) {
            throw EmptyError("linked list is empty");
        }
        const T value = tail_->value;
        unlink(tail_);
        return value;
    }

    const T& front() const {
        if (head_ == nullptr) {
            throw EmptyError("linked list is empty");
        }
        return head_->value;
    }

    const T& back() const {
        if (tail_ == nullptr) {
            throw EmptyError("linked list is empty");
        }
        return tail_->value;
    }

    void eraseAt(std::size_t index) {
        checkIndex(index);
        unlink(nodeAt(index));
    }

    bool eraseValue(const T& value) {
        for (Node* cur = head_; cur != nullptr; cur = cur->next) {
            if (cur->value == value) {
                unlink(cur);
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

    std::vector<T> toReversedVector() const {
        std::vector<T> items;
        items.reserve(size_);
        for (const Node* cur = tail_; cur != nullptr; cur = cur->prev) {
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

    std::string toReversedString() const {
        const std::vector<T> items = toReversedVector();
        return joinValues(items.begin(), items.end());
    }

    std::string kind() const override { return "LinkedList"; }

    void save(BinaryWriter& writer) const override {
        writer.writeTag("LLST");
        writer.writeU64(size_);
        for (const Node* cur = head_; cur != nullptr; cur = cur->next) {
            writer.writePod(cur->value);
        }
    }

    void load(BinaryReader& reader) override {
        reader.expectTag("LLST");
        const std::uint64_t count = reader.readCount();
        LinkedList fresh;
        for (std::uint64_t i = 0; i < count; ++i) {
            fresh.pushBack(reader.readPod<T>());
        }
        swap(fresh);
    }

private:
    struct Node {
        T value;
        Node* prev;
        Node* next;
    };

    void checkIndex(std::size_t index) const {
        if (index >= size_) {
            throw IndexError("index out of range: " + std::to_string(index));
        }
    }

    Node* nodeAt(std::size_t index) const {
        if (index < size_ / 2) {
            Node* cur = head_;
            for (std::size_t i = 0; i < index; ++i) {
                cur = cur->next;
            }
            return cur;
        }
        Node* cur = tail_;
        for (std::size_t i = size_ - 1; i > index; --i) {
            cur = cur->prev;
        }
        return cur;
    }

    void unlink(Node* node) {
        if (node->prev != nullptr) {
            node->prev->next = node->next;
        } else {
            head_ = node->next;
        }
        if (node->next != nullptr) {
            node->next->prev = node->prev;
        } else {
            tail_ = node->prev;
        }
        delete node;
        --size_;
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
