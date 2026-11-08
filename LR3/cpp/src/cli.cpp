#include "ds/cli.hpp"

#include <filesystem>
#include <ostream>

#include "ds/database.hpp"
#include "ds/errors.hpp"

namespace ds {

int runCli(const std::vector<std::string>& args, std::ostream& out, std::ostream& err) {
    std::string filename;
    std::string query;
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--file" && i + 1 < args.size()) {
            filename = args[++i];
        } else if (args[i] == "--query" && i + 1 < args.size()) {
            query = args[++i];
        }
    }

    if (query.empty()) {
        out << "Usage: dsdb --file <file.bin> --query '<COMMAND> [args]'\n";
        return 0;
    }

    Database database;
    try {
        if (!filename.empty() && std::filesystem::exists(filename)) {
            database.loadFromFile(filename);
        }
        const std::string reply = database.execute(query);
        out << reply << '\n';
        if (reply.rfind("ERROR:", 0) == 0) {
            return 1;
        }
        if (!filename.empty()) {
            database.saveToFile(filename);
        }
    } catch (const DsError& error) {
        err << "ERROR: " << error.what() << '\n';
        return 2;
    }
    return 0;
}

}  // namespace ds
