#include "ds/database.hpp"

#include <fstream>
#include <utility>

#include "ds/errors.hpp"

namespace ds {

namespace {

constexpr const char* kMagic = "DSDB";
constexpr std::uint8_t kVersion = 1;

int readInt(std::istringstream& args) {
    int value = 0;
    if (!(args >> value)) {
        throw FormatError("integer argument expected");
    }
    return value;
}

std::size_t readIndex(std::istringstream& args) {
    const int value = readInt(args);
    if (value < 0) {
        throw IndexError("index must not be negative: " + std::to_string(value));
    }
    return static_cast<std::size_t>(value);
}

std::string readWord(std::istringstream& args) {
    std::string word;
    if (!(args >> word)) {
        throw FormatError("key argument expected");
    }
    return word;
}

bool hasMore(std::istringstream& args) {
    return static_cast<bool>(args >> std::ws) && !args.eof();
}

std::string indexOrMinusOne(const std::optional<std::size_t>& index) {
    return index ? std::to_string(*index) : "-1";
}

std::string joinVector(const std::vector<int>& values) {
    return joinValues(values.begin(), values.end());
}

}  // namespace

std::string Database::execute(const std::string& query) {
    std::istringstream args(query);
    std::string command;
    if (!(args >> command)) {
        return "ERROR: empty query";
    }
    try {
        Reply reply = executeArray(command, args);
        if (!reply) {
            reply = executeForwardList(command, args);
        }
        if (!reply) {
            reply = executeLinkedList(command, args);
        }
        if (!reply) {
            reply = executeStack(command, args);
        }
        if (!reply) {
            reply = executeQueue(command, args);
        }
        if (!reply) {
            reply = executeTree(command, args);
        }
        if (!reply) {
            reply = executeHashTable(command, args);
        }
        if (!reply) {
            reply = executeCommon(command, args);
        }
        if (!reply) {
            return "Unknown command: " + command;
        }
        return *reply;
    } catch (const DsError& error) {
        return std::string("ERROR: ") + error.what();
    }
}

Database::Reply Database::executeArray(const std::string& command, std::istringstream& args) {
    if (command == "MPUSH") {
        const int first = readInt(args);
        if (hasMore(args)) {
            const int second = readInt(args);
            if (first < 0) {
                throw IndexError("index must not be negative: " + std::to_string(first));
            }
            array_.insert(static_cast<std::size_t>(first), second);
            return std::to_string(second);
        }
        array_.pushBack(first);
        return std::to_string(first);
    }
    if (command == "MGET") {
        return std::to_string(array_.get(readIndex(args)));
    }
    if (command == "MSET") {
        const std::size_t index = readIndex(args);
        array_.set(index, readInt(args));
        return "OK";
    }
    if (command == "MDEL") {
        array_.erase(readIndex(args));
        return "OK";
    }
    if (command == "MLEN") {
        return std::to_string(array_.size());
    }
    return std::nullopt;
}

Database::Reply Database::executeForwardList(const std::string& command, std::istringstream& args) {
    if (command == "FPUSH") {
        const int first = readInt(args);
        if (hasMore(args)) {
            if (first < 0) {
                throw IndexError("index must not be negative: " + std::to_string(first));
            }
            forwardList_.insert(static_cast<std::size_t>(first), readInt(args));
        } else {
            forwardList_.pushFront(first);
        }
        return "OK";
    }
    if (command == "FPUSH_END") {
        forwardList_.pushBack(readInt(args));
        return "OK";
    }
    if (command == "FPUSH_AFTER") {
        const std::size_t index = readIndex(args);
        forwardList_.insertAfter(index, readInt(args));
        return "OK";
    }
    if (command == "FGET") {
        return std::to_string(forwardList_.get(readIndex(args)));
    }
    if (command == "FDEL") {
        forwardList_.eraseAt(readIndex(args));
        return "OK";
    }
    if (command == "FDEL_VALUE") {
        return forwardList_.eraseValue(readInt(args)) ? "OK" : "NOT_FOUND";
    }
    if (command == "FSEARCH") {
        return indexOrMinusOne(forwardList_.indexOf(readInt(args)));
    }
    return std::nullopt;
}

Database::Reply Database::executeLinkedList(const std::string& command, std::istringstream& args) {
    if (command == "LPUSH") {
        const int first = readInt(args);
        if (hasMore(args)) {
            if (first < 0) {
                throw IndexError("index must not be negative: " + std::to_string(first));
            }
            linkedList_.insert(static_cast<std::size_t>(first), readInt(args));
        } else {
            linkedList_.pushFront(first);
        }
        return "OK";
    }
    if (command == "LPUSH_END") {
        linkedList_.pushBack(readInt(args));
        return "OK";
    }
    if (command == "LPUSH_AFTER") {
        const std::size_t index = readIndex(args);
        linkedList_.insertAfter(index, readInt(args));
        return "OK";
    }
    if (command == "LGET") {
        return std::to_string(linkedList_.get(readIndex(args)));
    }
    if (command == "LDEL") {
        linkedList_.eraseAt(readIndex(args));
        return "OK";
    }
    if (command == "LDEL_VALUE") {
        return linkedList_.eraseValue(readInt(args)) ? "OK" : "NOT_FOUND";
    }
    if (command == "LSEARCH") {
        return indexOrMinusOne(linkedList_.indexOf(readInt(args)));
    }
    return std::nullopt;
}

Database::Reply Database::executeStack(const std::string& command, std::istringstream& args) {
    if (command == "SPUSH") {
        const int value = readInt(args);
        stack_.push(value);
        return std::to_string(value);
    }
    if (command == "SPOP") {
        return std::to_string(stack_.pop());
    }
    if (command == "SPEEK") {
        return std::to_string(stack_.peek());
    }
    return std::nullopt;
}

Database::Reply Database::executeQueue(const std::string& command, std::istringstream& args) {
    if (command == "QPUSH") {
        const int value = readInt(args);
        queue_.push(value);
        return std::to_string(value);
    }
    if (command == "QPOP") {
        return std::to_string(queue_.pop());
    }
    if (command == "QPEEK") {
        return std::to_string(queue_.peek());
    }
    return std::nullopt;
}

Database::Reply Database::executeTree(const std::string& command, std::istringstream& args) {
    if (command == "TINSERT") {
        const int value = readInt(args);
        tree_.insert(value);
        return std::to_string(value);
    }
    if (command == "TGET") {
        return std::to_string(tree_.get(readIndex(args)));
    }
    if (command == "TSEARCH") {
        return indexOrMinusOne(tree_.indexOf(readInt(args)));
    }
    if (command == "TDEL") {
        tree_.erase(readIndex(args));
        return "OK";
    }
    if (command == "TFULL") {
        return tree_.isFull() ? "TRUE" : "FALSE";
    }
    if (command == "THEIGHT") {
        return std::to_string(tree_.height());
    }
    if (command == "TORDER") {
        const std::string kind = readWord(args);
        if (kind == "pre") {
            return joinVector(tree_.preorder());
        }
        if (kind == "in") {
            return joinVector(tree_.inorder());
        }
        if (kind == "post") {
            return joinVector(tree_.postorder());
        }
        if (kind == "level") {
            return joinVector(tree_.levelOrder());
        }
        throw FormatError("unknown traversal: " + kind);
    }
    return std::nullopt;
}

Database::Reply Database::executeHashTable(const std::string& command, std::istringstream& args) {
    if (command == "HPUT") {
        const std::string key = readWord(args);
        hashTable_.put(key, readInt(args));
        return "OK";
    }
    if (command == "HGET") {
        const std::optional<int> value = hashTable_.get(readWord(args));
        return value ? std::to_string(*value) : "NOT_FOUND";
    }
    if (command == "HDEL") {
        return hashTable_.erase(readWord(args)) ? "TRUE" : "FALSE";
    }
    if (command == "HHAS") {
        return hashTable_.contains(readWord(args)) ? "TRUE" : "FALSE";
    }
    if (command == "HSIZE") {
        return std::to_string(hashTable_.size());
    }
    return std::nullopt;
}

Database::Reply Database::executeCommon(const std::string& command, std::istringstream& args) const {
    if (command == "PRINT") {
        const std::string what = readWord(args);
        if (what == "M") {
            return array_.toString();
        }
        if (what == "F") {
            return forwardList_.toString();
        }
        if (what == "L") {
            return linkedList_.toString();
        }
        if (what == "LR") {
            return linkedList_.toReversedString();
        }
        if (what == "S") {
            return stack_.toString();
        }
        if (what == "Q") {
            return queue_.toString();
        }
        if (what == "T") {
            return tree_.toString();
        }
        if (what == "H") {
            return hashTable_.toString();
        }
        throw FormatError("unknown container: " + what);
    }
    if (command == "STATS") {
        std::string report;
        for (const Container* container : containers()) {
            if (!report.empty()) {
                report += '\n';
            }
            report += container->kind() + ": " + std::to_string(container->size());
        }
        return report;
    }
    return std::nullopt;
}

std::vector<const Container*> Database::containers() const {
    return {&array_, &forwardList_, &linkedList_, &stack_, &queue_, &tree_, &hashTable_};
}

void Database::save(BinaryWriter& writer) const {
    writer.writeTag(kMagic);
    writer.writePod(kVersion);
    const std::vector<const Container*> all = containers();
    for (const ISerializable* item : all) {
        item->save(writer);
    }
}

void Database::load(BinaryReader& reader) {
    reader.expectTag(kMagic);
    const auto version = reader.readPod<std::uint8_t>();
    if (version != kVersion) {
        throw FormatError("unsupported format version: " + std::to_string(version));
    }
    Database fresh;
    fresh.array_.load(reader);
    fresh.forwardList_.load(reader);
    fresh.linkedList_.load(reader);
    fresh.stack_.load(reader);
    fresh.queue_.load(reader);
    fresh.tree_.load(reader);
    fresh.hashTable_.load(reader);
    *this = std::move(fresh);
}

void Database::saveToFile(const std::string& path) const {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        throw DsError("cannot open file for writing: " + path);
    }
    BinaryWriter writer(out);
    save(writer);
}

void Database::loadFromFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) {
        throw DsError("cannot open file for reading: " + path);
    }
    BinaryReader reader(in);
    load(reader);
}

}  // namespace ds
