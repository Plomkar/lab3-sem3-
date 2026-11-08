#pragma once

#include <cstddef>
#include <initializer_list>
#include <string>

#include "ds/container.hpp"
#include "ds/errors.hpp"
#include "ds/linked_list.hpp"

namespace ds {

template <typename T>
class Queue : public Container {
public:
    Queue() = default;

    Queue(std::initializer_list<T> items) {
        for (const T& item : items) {
            push(item);
        }
    }


    void push(const T& value) { items_.pushBack(value); }

    T pop() {
        if (items_.empty()) {
            throw EmptyError("queue is empty");
        }
        return items_.popFront();
    }

    const T& peek() const {
        if (items_.empty()) {
            throw EmptyError("queue is empty");
        }
        return items_.front();
    }

    std::size_t size() const noexcept override { return items_.size(); }

    void clear() noexcept override { items_.clear(); }

    std::string toString() const override { return items_.toString(); }

    std::string kind() const override { return "Queue"; }

    void save(BinaryWriter& writer) const override {
        writer.writeTag("QUEU");
        items_.save(writer);
    }

    void load(BinaryReader& reader) override {
        reader.expectTag("QUEU");
        items_.load(reader);
    }

private:
    LinkedList<T> items_;
};

}  // namespace ds
