#include <iostream>
#include <string>
#include <vector>

#include "ds/cli.hpp"

int main(int argc, char* argv[]) {
    const std::vector<std::string> args(argv + 1, argv + argc);
    return ds::runCli(args, std::cout, std::cerr);
}
