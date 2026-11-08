#pragma once

#include <cstddef>
#include <string>

#include "ds/serialization.hpp"

namespace ds {

class ISerializable {
public:
    virtual ~ISerializable() = default;
    virtual void save(BinaryWriter& writer) const = 0;
    virtual void load(BinaryReader& reader) = 0;
};

class Container : public ISerializable {
public:
    ~Container() override = default;

    virtual std::size_t size() const noexcept = 0;
    virtual void clear() noexcept = 0;
    virtual std::string toString() const = 0;
    virtual std::string kind() const = 0;

    bool empty() const noexcept { return size() == 0; }

protected:
    Container() = default;
    Container(const Container&) = default;
    Container(Container&&) noexcept = default;
    Container& operator=(const Container&) = default;
    Container& operator=(Container&&) noexcept = default;
};

}  // namespace ds
