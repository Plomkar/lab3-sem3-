#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <string>
#include <type_traits>

namespace ds {

class BinaryWriter {
public:
    explicit BinaryWriter(std::ostream& out);

    void writeBytes(const void* data, std::size_t size);
    void writeU64(std::uint64_t value);
    void writeString(const std::string& value);
    void writeTag(const std::string& tag);

    template <typename T>
    void writePod(const T& value) {
        static_assert(std::is_trivially_copyable_v<T>, "writePod requires a trivially copyable type");
        writeBytes(&value, sizeof(T));
    }

private:
    std::ostream& out_;
};

class BinaryReader {
public:
    explicit BinaryReader(std::istream& in);

    void readBytes(void* data, std::size_t size);
    std::uint64_t readU64();
    std::uint64_t readCount();
    std::string readString();
    void expectTag(const std::string& tag);

    template <typename T>
    T readPod() {
        static_assert(std::is_trivially_copyable_v<T>, "readPod requires a trivially copyable type");
        T value{};
        readBytes(&value, sizeof(T));
        return value;
    }

private:
    std::istream& in_;
};

}  // namespace ds
