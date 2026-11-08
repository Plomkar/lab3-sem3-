#include "ds/serialization.hpp"

#include <array>
#include <istream>
#include <ostream>

#include "ds/errors.hpp"

namespace ds {

namespace {

constexpr std::uint64_t kMaxCount = 1ULL << 28U;
constexpr std::uint64_t kMaxStringLength = 1ULL << 24U;

}  // namespace

BinaryWriter::BinaryWriter(std::ostream& out) : out_(out) {}

void BinaryWriter::writeBytes(const void* data, std::size_t size) {
    out_.write(static_cast<const char*>(data), static_cast<std::streamsize>(size));
    if (!out_) {
        throw DsError("binary write failed");
    }
}

void BinaryWriter::writeU64(std::uint64_t value) {
    std::array<unsigned char, 8> bytes{};
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        bytes[i] = static_cast<unsigned char>((value >> (8U * i)) & 0xFFU);
    }
    writeBytes(bytes.data(), bytes.size());
}

void BinaryWriter::writeString(const std::string& value) {
    writeU64(value.size());
    writeBytes(value.data(), value.size());
}

void BinaryWriter::writeTag(const std::string& tag) {
    writeBytes(tag.data(), tag.size());
}

BinaryReader::BinaryReader(std::istream& in) : in_(in) {}

void BinaryReader::readBytes(void* data, std::size_t size) {
    in_.read(static_cast<char*>(data), static_cast<std::streamsize>(size));
    if (static_cast<std::size_t>(in_.gcount()) != size) {
        throw FormatError("unexpected end of binary data");
    }
}

std::uint64_t BinaryReader::readU64() {
    std::array<unsigned char, 8> bytes{};
    readBytes(bytes.data(), bytes.size());
    std::uint64_t value = 0;
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        value |= static_cast<std::uint64_t>(bytes[i]) << (8U * i);
    }
    return value;
}

std::uint64_t BinaryReader::readCount() {
    const std::uint64_t count = readU64();
    if (count > kMaxCount) {
        throw FormatError("element count is too large: " + std::to_string(count));
    }
    return count;
}

std::string BinaryReader::readString() {
    const std::uint64_t length = readU64();
    if (length > kMaxStringLength) {
        throw FormatError("string is too long: " + std::to_string(length));
    }
    std::string value(static_cast<std::size_t>(length), '\0');
    readBytes(value.data(), value.size());
    return value;
}

void BinaryReader::expectTag(const std::string& tag) {
    std::string actual(tag.size(), '\0');
    readBytes(actual.data(), actual.size());
    if (actual != tag) {
        throw FormatError("unexpected block tag, expected " + tag);
    }
}

}  // namespace ds
