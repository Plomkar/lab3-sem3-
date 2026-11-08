#pragma once

#include <iosfwd>
#include <string>
#include <vector>

namespace ds {

int runCli(const std::vector<std::string>& args, std::ostream& out, std::ostream& err);

}  // namespace ds
