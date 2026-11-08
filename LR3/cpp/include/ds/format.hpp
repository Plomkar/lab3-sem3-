#pragma once

#include <sstream>
#include <string>

namespace ds {

template <typename T>
std::string formatValue(const T& value) {
    std::ostringstream stream;
    stream << value;
    return stream.str();
}

template <typename Iterator>
std::string joinValues(Iterator first, Iterator last, const std::string& separator = " ") {
    std::string result;
    for (Iterator it = first; it != last; ++it) {
        if (it != first) {
            result += separator;
        }
        result += formatValue(*it);
    }
    return result;
}

}  // namespace ds
