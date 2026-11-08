#pragma once

#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "ds/array.hpp"
#include "ds/complete_binary_tree.hpp"
#include "ds/container.hpp"
#include "ds/forward_list.hpp"
#include "ds/hash_table.hpp"
#include "ds/linked_list.hpp"
#include "ds/queue.hpp"
#include "ds/stack.hpp"

namespace ds {

class Database {
public:
    Database() = default;

    std::string execute(const std::string& query);

    void save(BinaryWriter& writer) const;
    void load(BinaryReader& reader);
    void saveToFile(const std::string& path) const;
    void loadFromFile(const std::string& path);

    std::vector<const Container*> containers() const;

    const Array<int>& array() const noexcept { return array_; }
    const ForwardList<int>& forwardList() const noexcept { return forwardList_; }
    const LinkedList<int>& linkedList() const noexcept { return linkedList_; }
    const Stack<int>& stack() const noexcept { return stack_; }
    const Queue<int>& queue() const noexcept { return queue_; }
    const CompleteBinaryTree<int>& tree() const noexcept { return tree_; }
    const HashTable<int>& hashTable() const noexcept { return hashTable_; }

private:
    using Reply = std::optional<std::string>;

    Reply executeArray(const std::string& command, std::istringstream& args);
    Reply executeForwardList(const std::string& command, std::istringstream& args);
    Reply executeLinkedList(const std::string& command, std::istringstream& args);
    Reply executeStack(const std::string& command, std::istringstream& args);
    Reply executeQueue(const std::string& command, std::istringstream& args);
    Reply executeTree(const std::string& command, std::istringstream& args);
    Reply executeHashTable(const std::string& command, std::istringstream& args);
    Reply executeCommon(const std::string& command, std::istringstream& args) const;

    Array<int> array_;
    ForwardList<int> forwardList_;
    LinkedList<int> linkedList_;
    Stack<int> stack_;
    Queue<int> queue_;
    CompleteBinaryTree<int> tree_;
    HashTable<int> hashTable_;
};

}  // namespace ds
