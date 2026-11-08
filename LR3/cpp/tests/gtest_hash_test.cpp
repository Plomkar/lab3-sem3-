#include <gtest/gtest.h>

#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "ds/errors.hpp"
#include "ds/hash_table.hpp"
#include "ds/hashes.hpp"

TEST(Hashes, PolynomialHashIsDeterministic) {
    EXPECT_EQ(ds::polynomialHash("abc"), ds::polynomialHash("abc"));
    EXPECT_NE(ds::polynomialHash("abc"), ds::polynomialHash("abd"));
    EXPECT_EQ(ds::polynomialHash(""), 0ULL);
    EXPECT_EQ(ds::polynomialHash("abc"), 1677554ULL);
}

TEST(Hashes, Fnv1aHashKnownValues) {
    EXPECT_EQ(ds::fnv1aHash(""), 14695981039346656037ULL);
    EXPECT_EQ(ds::fnv1aHash("a"), 12638187200555641996ULL);
    EXPECT_NE(ds::fnv1aHash("a"), ds::fnv1aHash("b"));
}

TEST(Hashes, PrimeDetection) {
    EXPECT_FALSE(ds::isPrime(0));
    EXPECT_FALSE(ds::isPrime(1));
    EXPECT_TRUE(ds::isPrime(2));
    EXPECT_TRUE(ds::isPrime(13));
    EXPECT_FALSE(ds::isPrime(15));
    EXPECT_TRUE(ds::isPrime(1009));
}

TEST(Hashes, NextPrime) {
    EXPECT_EQ(ds::nextPrime(0), 2U);
    EXPECT_EQ(ds::nextPrime(11), 11U);
    EXPECT_EQ(ds::nextPrime(12), 13U);
    EXPECT_EQ(ds::nextPrime(1000), 1009U);
}

TEST(HashTableBasics, NewTableIsEmpty) {
    ds::HashTable<int> table;
    EXPECT_TRUE(table.empty());
    EXPECT_EQ(table.kind(), "HashTable");
    EXPECT_EQ(table.capacity(), 11U);
    EXPECT_EQ(table.strategy(), ds::ProbeStrategy::Double);
    EXPECT_FALSE(table.get("missing").has_value());
}

TEST(HashTableBasics, CapacityIsAlwaysPrime) {
    ds::HashTable<int> table(ds::ProbeStrategy::Linear, 20);
    EXPECT_EQ(table.capacity(), 23U);
    ds::HashTable<int> tiny(ds::ProbeStrategy::Linear, 0);
    EXPECT_EQ(tiny.capacity(), 3U);
}

TEST(HashTableBasics, PutGetAndUpdate) {
    ds::HashTable<int> table;
    table.put("a", 1);
    table.put("b", 2);
    table.put("a", 10);
    EXPECT_EQ(table.size(), 2U);
    EXPECT_EQ(table.get("a").value(), 10);
    EXPECT_EQ(table.get("b").value(), 2);
    EXPECT_TRUE(table.contains("a"));
    EXPECT_FALSE(table.contains("c"));
}

TEST(HashTableBasics, EraseAndReinsert) {
    ds::HashTable<int> table;
    table.put("x", 1);
    EXPECT_TRUE(table.erase("x"));
    EXPECT_FALSE(table.erase("x"));
    EXPECT_FALSE(table.contains("x"));
    EXPECT_EQ(table.size(), 0U);
    table.put("x", 2);
    EXPECT_EQ(table.get("x").value(), 2);
}

TEST(HashTableBasics, KeysAreSortedAndToStringIsStable) {
    ds::HashTable<int> table;
    table.put("b", 2);
    table.put("a", 1);
    table.put("c", 3);
    const std::vector<std::string> keys = table.keys();
    ASSERT_EQ(keys.size(), 3U);
    EXPECT_EQ(keys[0], "a");
    EXPECT_EQ(keys[2], "c");
    EXPECT_EQ(table.toString(), "a=1 b=2 c=3");
}

TEST(HashTableBasics, ClearRemovesEverything) {
    ds::HashTable<int> table;
    table.put("a", 1);
    table.put("b", 2);
    table.clear();
    EXPECT_TRUE(table.empty());
    EXPECT_FALSE(table.contains("a"));
    EXPECT_DOUBLE_EQ(table.loadFactor(), 0.0);
    table.put("a", 5);
    EXPECT_EQ(table.get("a").value(), 5);
}

TEST(HashTableBasics, CopyAndMoveKeepContent) {
    ds::HashTable<int> table;
    table.put("a", 1);
    ds::HashTable<int> copy = table;
    copy.put("b", 2);
    EXPECT_EQ(table.size(), 1U);
    EXPECT_EQ(copy.size(), 2U);
    ds::HashTable<int> moved = std::move(copy);
    EXPECT_EQ(moved.size(), 2U);
}

TEST(HashTableGrowth, RehashKeepsAllKeysDouble) {
    ds::HashTable<int> table(ds::ProbeStrategy::Double, 5);
    const std::size_t initial = table.capacity();
    for (int i = 0; i < 500; ++i) {
        table.put("key" + std::to_string(i), i);
    }
    EXPECT_EQ(table.size(), 500U);
    EXPECT_GT(table.capacity(), initial);
    EXPECT_LE(table.loadFactor(), 0.7 + 1e-9);
    for (int i = 0; i < 500; ++i) {
        ASSERT_TRUE(table.contains("key" + std::to_string(i)));
        EXPECT_EQ(table.get("key" + std::to_string(i)).value(), i);
    }
}

TEST(HashTableGrowth, RehashKeepsAllKeysLinear) {
    ds::HashTable<int> table(ds::ProbeStrategy::Linear, 5);
    for (int i = 0; i < 500; ++i) {
        table.put("item" + std::to_string(i), i * 2);
    }
    EXPECT_EQ(table.size(), 500U);
    for (int i = 0; i < 500; ++i) {
        EXPECT_EQ(table.get("item" + std::to_string(i)).value(), i * 2);
    }
}

TEST(HashTableGrowth, TombstonesAreCleanedByRehash) {
    ds::HashTable<int> table(ds::ProbeStrategy::Double, 11);
    for (int round = 0; round < 50; ++round) {
        for (int i = 0; i < 5; ++i) {
            table.put("t" + std::to_string(i), round);
        }
        for (int i = 0; i < 5; ++i) {
            EXPECT_TRUE(table.erase("t" + std::to_string(i)));
        }
        table.put("r" + std::to_string(round), round);
    }
    EXPECT_EQ(table.size(), 50U);
    for (int round = 0; round < 50; ++round) {
        EXPECT_EQ(table.get("r" + std::to_string(round)).value(), round);
    }
}

TEST(HashTableGrowth, ErasedSlotsAreReusedBeforeRehash) {
    ds::HashTable<int> table(ds::ProbeStrategy::Linear, 11);
    table.put("a", 1);
    table.put("b", 2);
    EXPECT_TRUE(table.erase("a"));
    table.put("a", 3);
    EXPECT_EQ(table.get("a").value(), 3);
    EXPECT_EQ(table.size(), 2U);
}

TEST(HashTableSerialization, RoundTripPreservesData) {
    ds::HashTable<int> table(ds::ProbeStrategy::Linear);
    for (int i = 0; i < 40; ++i) {
        table.put("k" + std::to_string(i), i);
    }
    table.erase("k7");
    std::stringstream buffer;
    ds::BinaryWriter writer(buffer);
    table.save(writer);
    ds::BinaryReader reader(buffer);
    ds::HashTable<int> loaded;
    loaded.put("old", 1);
    loaded.load(reader);
    EXPECT_EQ(loaded.strategy(), ds::ProbeStrategy::Linear);
    EXPECT_EQ(loaded.size(), 39U);
    EXPECT_FALSE(loaded.contains("old"));
    EXPECT_FALSE(loaded.contains("k7"));
    EXPECT_EQ(loaded.get("k39").value(), 39);
}

TEST(HashTableSerialization, RejectsWrongTag) {
    std::stringstream buffer;
    ds::BinaryWriter writer(buffer);
    writer.writeTag("NOPE");
    ds::BinaryReader reader(buffer);
    ds::HashTable<int> table;
    EXPECT_THROW(table.load(reader), ds::FormatError);
}

TEST(HashTableSerialization, RejectsUnknownStrategy) {
    std::stringstream buffer;
    ds::BinaryWriter writer(buffer);
    writer.writeTag("HASH");
    writer.writePod(static_cast<std::uint8_t>(9));
    writer.writeU64(0);
    ds::BinaryReader reader(buffer);
    ds::HashTable<int> table;
    EXPECT_THROW(table.load(reader), ds::FormatError);
}

TEST(HashTableCollisions, DifferentStrategiesGiveSameResults) {
    ds::HashTable<int> linear(ds::ProbeStrategy::Linear, 7);
    ds::HashTable<int> doubled(ds::ProbeStrategy::Double, 7);
    std::set<std::string> keys;
    for (int i = 0; i < 100; ++i) {
        const std::string key = "collide" + std::to_string(i % 37);
        keys.insert(key);
        linear.put(key, i);
        doubled.put(key, i);
    }
    EXPECT_EQ(linear.size(), keys.size());
    EXPECT_EQ(doubled.size(), keys.size());
    for (const std::string& key : keys) {
        EXPECT_EQ(linear.get(key).value(), doubled.get(key).value());
    }
}
