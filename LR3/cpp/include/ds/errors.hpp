#pragma once

#include <stdexcept>
#include <string>

namespace ds {

class DsError : public std::runtime_error {
public:
    explicit DsError(const std::string& message) : std::runtime_error(message) {}
};

class IndexError : public DsError {
public:
    explicit IndexError(const std::string& message) : DsError(message) {}
};

class EmptyError : public DsError {
public:
    explicit EmptyError(const std::string& message) : DsError(message) {}
};

class FormatError : public DsError {
public:
    explicit FormatError(const std::string& message) : DsError(message) {}
};

}  // namespace ds
