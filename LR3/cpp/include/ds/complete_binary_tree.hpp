#pragma once

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

namespace ds {

template <typename T>
class CompleteBinaryTree : public Container {
    static_assert(std::is_trivially_copyable_v<T>, "CompleteBinaryTree requires a trivially copyable element type");

public:
    CompleteBinaryTree() = default;


    void insert(const T& value) { nodes_.push_back(value); }

    const T& get(std::size_t index) const {
        checkIndex(index);
        return nodes_[index];
    }

    std::optional<std::size_t> indexOf(const T& value) const {
        for (std::size_t i = 0; i < nodes_.size(); ++i) {
            if (nodes_[i] == value) {
                return i;
            }
        }
        return std::nullopt;
    }

    void erase(std::size_t index) {
        checkIndex(index);
        nodes_[index] = nodes_.back();
        nodes_.pop_back();
    }

    std::optional<std::size_t> parent(std::size_t index) const {
        checkIndex(index);
        if (index == 0) {
            return std::nullopt;
        }
        return (index - 1) / 2;
    }

    std::optional<std::size_t> leftChild(std::size_t index) const {
        checkIndex(index);
        const std::size_t child = 2 * index + 1;
        if (child >= nodes_.size()) {
            return std::nullopt;
        }
        return child;
    }

    std::optional<std::size_t> rightChild(std::size_t index) const {
        checkIndex(index);
        const std::size_t child = 2 * index + 2;
        if (child >= nodes_.size()) {
            return std::nullopt;
        }
        return child;
    }

    std::size_t height() const noexcept {
        std::size_t levels = 0;
        for (std::size_t remaining = nodes_.size(); remaining > 0; remaining >>= 1U) {
            ++levels;
        }
        return levels;
    }

    bool isFull() const noexcept {
        const std::size_t count = nodes_.size();
        return (count & (count + 1)) == 0;
    }

    std::vector<T> preorder() const {
        std::vector<T> result;
        preorderFrom(0, result);
        return result;
    }

    std::vector<T> inorder() const {
        std::vector<T> result;
        inorderFrom(0, result);
        return result;
    }

    std::vector<T> postorder() const {
        std::vector<T> result;
        postorderFrom(0, result);
        return result;
    }

    std::vector<T> levelOrder() const { return nodes_; }

    std::size_t size() const noexcept override { return nodes_.size(); }

    void clear() noexcept override { nodes_.clear(); }

    std::string toString() const override { return joinValues(nodes_.begin(), nodes_.end()); }

    std::string kind() const override { return "CompleteBinaryTree"; }

    void save(BinaryWriter& writer) const override {
        writer.writeTag("CBTR");
        writer.writeU64(nodes_.size());
        for (const T& value : nodes_) {
            writer.writePod(value);
        }
    }

    void load(BinaryReader& reader) override {
        reader.expectTag("CBTR");
        const std::uint64_t count = reader.readCount();
        std::vector<T> fresh;
        fresh.reserve(static_cast<std::size_t>(count));
        for (std::uint64_t i = 0; i < count; ++i) {
            fresh.push_back(reader.readPod<T>());
        }
        nodes_ = std::move(fresh);
    }

private:
    void checkIndex(std::size_t index) const {
        if (index >= nodes_.size()) {
            throw IndexError("index out of range: " + std::to_string(index));
        }
    }

    void preorderFrom(std::size_t index, std::vector<T>& out) const {
        if (index >= nodes_.size()) {
            return;
        }
        out.push_back(nodes_[index]);
        preorderFrom(2 * index + 1, out);
        preorderFrom(2 * index + 2, out);
    }

    void inorderFrom(std::size_t index, std::vector<T>& out) const {
        if (index >= nodes_.size()) {
            return;
        }
        inorderFrom(2 * index + 1, out);
        out.push_back(nodes_[index]);
        inorderFrom(2 * index + 2, out);
    }

    void postorderFrom(std::size_t index, std::vector<T>& out) const {
        if (index >= nodes_.size()) {
            return;
        }
        postorderFrom(2 * index + 1, out);
        postorderFrom(2 * index + 2, out);
        out.push_back(nodes_[index]);
    }

    std::vector<T> nodes_;
};

}  // namespace ds
