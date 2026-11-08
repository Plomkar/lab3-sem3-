#include <catch2/catch_test_macros.hpp>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "ds/cli.hpp"
#include "ds/database.hpp"
#include "ds/errors.hpp"

namespace {

std::string tempPath(const std::string& name) {
    return (std::filesystem::temp_directory_path() / ("ds_lab3_" + name + ".bin")).string();
}

}  // namespace

TEST_CASE("empty query is an error", "[database]") {
    ds::Database db;
    CHECK(db.execute("") == "ERROR: empty query");
    CHECK(db.execute("   ") == "ERROR: empty query");
}

TEST_CASE("unknown command is reported", "[database]") {
    ds::Database db;
    CHECK(db.execute("BOOM 1") == "Unknown command: BOOM");
}

TEST_CASE("array commands", "[database][array]") {
    ds::Database db;
    CHECK(db.execute("MPUSH 5") == "5");
    CHECK(db.execute("MPUSH 7") == "7");
    CHECK(db.execute("MPUSH 1 6") == "6");
    CHECK(db.execute("PRINT M") == "5 6 7");
    CHECK(db.execute("MGET 1") == "6");
    CHECK(db.execute("MSET 1 60") == "OK");
    CHECK(db.execute("MDEL 0") == "OK");
    CHECK(db.execute("MLEN") == "2");
    CHECK(db.execute("PRINT M") == "60 7");
    CHECK(db.execute("MGET 9").rfind("ERROR:", 0) == 0U);
    CHECK(db.execute("MPUSH -1 3").rfind("ERROR:", 0) == 0U);
    CHECK(db.execute("MGET -1").rfind("ERROR:", 0) == 0U);
    CHECK(db.execute("MGET").rfind("ERROR:", 0) == 0U);
}

TEST_CASE("forward list commands", "[database][list]") {
    ds::Database db;
    CHECK(db.execute("FPUSH 2") == "OK");
    CHECK(db.execute("FPUSH 1") == "OK");
    CHECK(db.execute("FPUSH_END 4") == "OK");
    CHECK(db.execute("FPUSH 2 3") == "OK");
    CHECK(db.execute("FPUSH_AFTER 3 40") == "OK");
    CHECK(db.execute("PRINT F") == "1 2 3 4 40");
    CHECK(db.execute("FGET 2") == "3");
    CHECK(db.execute("FSEARCH 4") == "3");
    CHECK(db.execute("FSEARCH 99") == "-1");
    CHECK(db.execute("FDEL 0") == "OK");
    CHECK(db.execute("FDEL_VALUE 40") == "OK");
    CHECK(db.execute("FDEL_VALUE 40") == "NOT_FOUND");
    CHECK(db.execute("PRINT F") == "2 3 4");
    CHECK(db.execute("FPUSH -1 5").rfind("ERROR:", 0) == 0U);
    CHECK(db.execute("FGET 10").rfind("ERROR:", 0) == 0U);
}

TEST_CASE("linked list commands", "[database][list]") {
    ds::Database db;
    CHECK(db.execute("LPUSH 2") == "OK");
    CHECK(db.execute("LPUSH 1") == "OK");
    CHECK(db.execute("LPUSH_END 4") == "OK");
    CHECK(db.execute("LPUSH 2 3") == "OK");
    CHECK(db.execute("LPUSH_AFTER 3 40") == "OK");
    CHECK(db.execute("PRINT L") == "1 2 3 4 40");
    CHECK(db.execute("PRINT LR") == "40 4 3 2 1");
    CHECK(db.execute("LGET 2") == "3");
    CHECK(db.execute("LSEARCH 4") == "3");
    CHECK(db.execute("LSEARCH 99") == "-1");
    CHECK(db.execute("LDEL 0") == "OK");
    CHECK(db.execute("LDEL_VALUE 40") == "OK");
    CHECK(db.execute("LDEL_VALUE 40") == "NOT_FOUND");
    CHECK(db.execute("PRINT L") == "2 3 4");
    CHECK(db.execute("LPUSH -1 5").rfind("ERROR:", 0) == 0U);
    CHECK(db.execute("LDEL 10").rfind("ERROR:", 0) == 0U);
}

TEST_CASE("stack commands", "[database][stack]") {
    ds::Database db;
    CHECK(db.execute("SPUSH 1") == "1");
    CHECK(db.execute("SPUSH 2") == "2");
    CHECK(db.execute("SPEEK") == "2");
    CHECK(db.execute("PRINT S") == "2 1");
    CHECK(db.execute("SPOP") == "2");
    CHECK(db.execute("SPOP") == "1");
    CHECK(db.execute("SPOP") == "ERROR: stack is empty");
    CHECK(db.execute("SPEEK") == "ERROR: stack is empty");
}

TEST_CASE("queue commands", "[database][queue]") {
    ds::Database db;
    CHECK(db.execute("QPUSH 1") == "1");
    CHECK(db.execute("QPUSH 2") == "2");
    CHECK(db.execute("QPEEK") == "1");
    CHECK(db.execute("PRINT Q") == "1 2");
    CHECK(db.execute("QPOP") == "1");
    CHECK(db.execute("QPOP") == "2");
    CHECK(db.execute("QPOP") == "ERROR: queue is empty");
    CHECK(db.execute("QPEEK") == "ERROR: queue is empty");
}

TEST_CASE("tree commands", "[database][tree]") {
    ds::Database db;
    for (int i = 1; i <= 7; ++i) {
        CHECK(db.execute("TINSERT " + std::to_string(i)) == std::to_string(i));
    }
    CHECK(db.execute("TGET 3") == "4");
    CHECK(db.execute("TSEARCH 5") == "4");
    CHECK(db.execute("TSEARCH 99") == "-1");
    CHECK(db.execute("TFULL") == "TRUE");
    CHECK(db.execute("THEIGHT") == "3");
    CHECK(db.execute("TORDER pre") == "1 2 4 5 3 6 7");
    CHECK(db.execute("TORDER in") == "4 2 5 1 6 3 7");
    CHECK(db.execute("TORDER post") == "4 5 2 6 7 3 1");
    CHECK(db.execute("TORDER level") == "1 2 3 4 5 6 7");
    CHECK(db.execute("TORDER zigzag").rfind("ERROR:", 0) == 0U);
    CHECK(db.execute("TDEL 0") == "OK");
    CHECK(db.execute("TFULL") == "FALSE");
    CHECK(db.execute("PRINT T") == "7 2 3 4 5 6");
    CHECK(db.execute("TGET 20").rfind("ERROR:", 0) == 0U);
}

TEST_CASE("hash table commands", "[database][hash]") {
    ds::Database db;
    CHECK(db.execute("HPUT apple 3") == "OK");
    CHECK(db.execute("HPUT pear 5") == "OK");
    CHECK(db.execute("HGET apple") == "3");
    CHECK(db.execute("HGET plum") == "NOT_FOUND");
    CHECK(db.execute("HHAS pear") == "TRUE");
    CHECK(db.execute("HHAS plum") == "FALSE");
    CHECK(db.execute("HSIZE") == "2");
    CHECK(db.execute("PRINT H") == "apple=3 pear=5");
    CHECK(db.execute("HDEL apple") == "TRUE");
    CHECK(db.execute("HDEL apple") == "FALSE");
    CHECK(db.execute("HPUT") .rfind("ERROR:", 0) == 0U);
}

TEST_CASE("print validates container name", "[database]") {
    ds::Database db;
    CHECK(db.execute("PRINT X").rfind("ERROR:", 0) == 0U);
    CHECK(db.execute("PRINT").rfind("ERROR:", 0) == 0U);
}

TEST_CASE("stats uses polymorphic containers", "[database]") {
    ds::Database db;
    db.execute("MPUSH 1");
    db.execute("SPUSH 2");
    db.execute("SPUSH 3");
    const std::string stats = db.execute("STATS");
    CHECK(stats.find("Array: 1") != std::string::npos);
    CHECK(stats.find("Stack: 2") != std::string::npos);
    CHECK(stats.find("HashTable: 0") != std::string::npos);
    CHECK(db.containers().size() == 7U);
}

TEST_CASE("accessors expose the containers", "[database]") {
    ds::Database db;
    db.execute("MPUSH 4");
    db.execute("FPUSH 5");
    db.execute("LPUSH 6");
    db.execute("SPUSH 7");
    db.execute("QPUSH 8");
    db.execute("TINSERT 9");
    db.execute("HPUT k 10");
    CHECK(db.array().size() == 1U);
    CHECK(db.forwardList().size() == 1U);
    CHECK(db.linkedList().size() == 1U);
    CHECK(db.stack().size() == 1U);
    CHECK(db.queue().size() == 1U);
    CHECK(db.tree().size() == 1U);
    CHECK(db.hashTable().size() == 1U);
}

TEST_CASE("database binary round trip in memory", "[database][serialization]") {
    ds::Database source;
    source.execute("MPUSH 1");
    source.execute("MPUSH 2");
    source.execute("FPUSH 3");
    source.execute("LPUSH_END 4");
    source.execute("SPUSH 5");
    source.execute("QPUSH 6");
    source.execute("TINSERT 7");
    source.execute("HPUT key 8");

    std::stringstream buffer;
    ds::BinaryWriter writer(buffer);
    source.save(writer);

    ds::Database loaded;
    loaded.execute("MPUSH 99");
    ds::BinaryReader reader(buffer);
    loaded.load(reader);
    CHECK(loaded.execute("PRINT M") == "1 2");
    CHECK(loaded.execute("PRINT F") == "3");
    CHECK(loaded.execute("PRINT L") == "4");
    CHECK(loaded.execute("PRINT S") == "5");
    CHECK(loaded.execute("PRINT Q") == "6");
    CHECK(loaded.execute("PRINT T") == "7");
    CHECK(loaded.execute("HGET key") == "8");
}

TEST_CASE("database load rejects bad magic and version", "[database][serialization]") {
    {
        std::stringstream buffer;
        ds::BinaryWriter writer(buffer);
        writer.writeTag("ZZZZ");
        ds::BinaryReader reader(buffer);
        ds::Database db;
        REQUIRE_THROWS_AS(db.load(reader), ds::FormatError);
    }
    {
        std::stringstream buffer;
        ds::BinaryWriter writer(buffer);
        writer.writeTag("DSDB");
        writer.writePod(static_cast<std::uint8_t>(99));
        ds::BinaryReader reader(buffer);
        ds::Database db;
        REQUIRE_THROWS_AS(db.load(reader), ds::FormatError);
    }
}

TEST_CASE("database file round trip", "[database][serialization]") {
    const std::string path = tempPath("roundtrip");
    ds::Database source;
    source.execute("MPUSH 10");
    source.execute("HPUT a 1");
    source.saveToFile(path);

    ds::Database loaded;
    loaded.loadFromFile(path);
    CHECK(loaded.execute("MGET 0") == "10");
    CHECK(loaded.execute("HGET a") == "1");
    std::remove(path.c_str());
}

TEST_CASE("database file errors", "[database][serialization]") {
    ds::Database db;
    REQUIRE_THROWS_AS(db.loadFromFile(tempPath("does_not_exist")), ds::DsError);
    REQUIRE_THROWS_AS(db.saveToFile("/nonexistent_dir_ds_lab3/file.bin"), ds::DsError);
}

TEST_CASE("cli prints usage without query", "[cli]") {
    std::ostringstream out;
    std::ostringstream err;
    CHECK(ds::runCli({}, out, err) == 0);
    CHECK(out.str().find("Usage") != std::string::npos);
}

TEST_CASE("cli runs queries against a file", "[cli]") {
    const std::string path = tempPath("cli");
    std::remove(path.c_str());
    std::ostringstream out;
    std::ostringstream err;
    CHECK(ds::runCli({"--file", path, "--query", "MPUSH 5"}, out, err) == 0);
    CHECK(ds::runCli({"--file", path, "--query", "MPUSH 6"}, out, err) == 0);
    std::ostringstream second;
    CHECK(ds::runCli({"--file", path, "--query", "PRINT M"}, second, err) == 0);
    CHECK(second.str() == "5 6\n");
    std::remove(path.c_str());
}

TEST_CASE("cli works without a file", "[cli]") {
    std::ostringstream out;
    std::ostringstream err;
    CHECK(ds::runCli({"--query", "MPUSH 1"}, out, err) == 0);
    CHECK(out.str() == "1\n");
}

TEST_CASE("cli reports query errors", "[cli]") {
    std::ostringstream out;
    std::ostringstream err;
    CHECK(ds::runCli({"--query", "SPOP"}, out, err) == 1);
    CHECK(out.str() == "ERROR: stack is empty\n");
}

TEST_CASE("cli reports corrupted files", "[cli]") {
    const std::string path = tempPath("corrupted");
    {
        std::ofstream broken(path, std::ios::binary);
        broken << "garbage data";
    }
    std::ostringstream out;
    std::ostringstream err;
    CHECK(ds::runCli({"--file", path, "--query", "MLEN"}, out, err) == 2);
    CHECK(err.str().find("ERROR") != std::string::npos);
    std::remove(path.c_str());
}

TEST_CASE("cli ignores a flag without value", "[cli]") {
    std::ostringstream out;
    std::ostringstream err;
    CHECK(ds::runCli({"--query"}, out, err) == 0);
    CHECK(out.str().find("Usage") != std::string::npos);
}
