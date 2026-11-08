#include <catch2/catch_test_macros.hpp>

#include <sstream>
#include <vector>

#include "ds/complete_binary_tree.hpp"
#include "ds/errors.hpp"

namespace {

ds::CompleteBinaryTree<int> makeTree(const std::vector<int>& values) {
    ds::CompleteBinaryTree<int> tree;
    for (const int value : values) {
        tree.insert(value);
    }
    return tree;
}

}  // namespace

TEST_CASE("new tree is empty", "[tree]") {
    ds::CompleteBinaryTree<int> tree;
    CHECK(tree.empty());
    CHECK(tree.height() == 0U);
    CHECK(tree.isFull());
    CHECK(tree.kind() == "CompleteBinaryTree");
    CHECK(tree.toString().empty());
}

TEST_CASE("insert fills levels left to right", "[tree]") {
    const auto tree = makeTree({1, 2, 3, 4, 5});
    CHECK(tree.size() == 5U);
    CHECK(tree.toString() == "1 2 3 4 5");
    CHECK(tree.get(0) == 1);
    CHECK(tree.get(4) == 5);
    REQUIRE_THROWS_AS(tree.get(5), ds::IndexError);
}

TEST_CASE("search returns index or nothing", "[tree]") {
    const auto tree = makeTree({7, 8, 9});
    const auto found = tree.indexOf(9);
    REQUIRE(found.has_value());
    CHECK(*found == 2U);
    CHECK_FALSE(tree.indexOf(100).has_value());
}

TEST_CASE("height grows with levels", "[tree]") {
    CHECK(makeTree({1}).height() == 1U);
    CHECK(makeTree({1, 2}).height() == 2U);
    CHECK(makeTree({1, 2, 3}).height() == 2U);
    CHECK(makeTree({1, 2, 3, 4}).height() == 3U);
    CHECK(makeTree({1, 2, 3, 4, 5, 6, 7}).height() == 3U);
    CHECK(makeTree({1, 2, 3, 4, 5, 6, 7, 8}).height() == 4U);
}

TEST_CASE("full tree detection", "[tree]") {
    CHECK(makeTree({1}).isFull());
    CHECK_FALSE(makeTree({1, 2}).isFull());
    CHECK(makeTree({1, 2, 3}).isFull());
    CHECK_FALSE(makeTree({1, 2, 3, 4, 5}).isFull());
    CHECK(makeTree({1, 2, 3, 4, 5, 6, 7}).isFull());
}

TEST_CASE("parent and children indices", "[tree]") {
    const auto tree = makeTree({1, 2, 3, 4, 5});
    CHECK_FALSE(tree.parent(0).has_value());
    CHECK(*tree.parent(1) == 0U);
    CHECK(*tree.parent(4) == 1U);
    CHECK(*tree.leftChild(0) == 1U);
    CHECK(*tree.rightChild(0) == 2U);
    CHECK(*tree.leftChild(1) == 3U);
    CHECK(*tree.rightChild(1) == 4U);
    CHECK_FALSE(tree.leftChild(2).has_value());
    CHECK_FALSE(tree.rightChild(2).has_value());
    REQUIRE_THROWS_AS(tree.parent(5), ds::IndexError);
    REQUIRE_THROWS_AS(tree.leftChild(5), ds::IndexError);
    REQUIRE_THROWS_AS(tree.rightChild(5), ds::IndexError);
}

TEST_CASE("depth first traversals", "[tree]") {
    const auto tree = makeTree({1, 2, 3, 4, 5, 6, 7});
    CHECK(tree.preorder() == std::vector<int>{1, 2, 4, 5, 3, 6, 7});
    CHECK(tree.inorder() == std::vector<int>{4, 2, 5, 1, 6, 3, 7});
    CHECK(tree.postorder() == std::vector<int>{4, 5, 2, 6, 7, 3, 1});
}

TEST_CASE("breadth first traversal equals storage order", "[tree]") {
    const auto tree = makeTree({5, 3, 8, 1});
    CHECK(tree.levelOrder() == std::vector<int>{5, 3, 8, 1});
}

TEST_CASE("traversals of empty tree are empty", "[tree]") {
    const ds::CompleteBinaryTree<int> tree;
    CHECK(tree.preorder().empty());
    CHECK(tree.inorder().empty());
    CHECK(tree.postorder().empty());
    CHECK(tree.levelOrder().empty());
}

TEST_CASE("erase keeps the tree complete", "[tree]") {
    auto tree = makeTree({1, 2, 3, 4, 5});
    tree.erase(1);
    CHECK(tree.toString() == "1 5 3 4");
    tree.erase(3);
    CHECK(tree.toString() == "1 5 3");
    REQUIRE_THROWS_AS(tree.erase(3), ds::IndexError);
    tree.erase(0);
    tree.erase(0);
    tree.erase(0);
    CHECK(tree.empty());
}

TEST_CASE("clear removes all nodes", "[tree]") {
    auto tree = makeTree({1, 2, 3});
    tree.clear();
    CHECK(tree.empty());
}

TEST_CASE("binary round trip", "[tree][serialization]") {
    const auto source = makeTree({9, 8, 7, 6});
    std::stringstream buffer;
    ds::BinaryWriter writer(buffer);
    source.save(writer);
    ds::BinaryReader reader(buffer);
    auto loaded = makeTree({1});
    loaded.load(reader);
    CHECK(loaded.toString() == "9 8 7 6");
}

TEST_CASE("load rejects wrong tag", "[tree][serialization]") {
    std::stringstream buffer;
    ds::BinaryWriter writer(buffer);
    writer.writeTag("NOPE");
    ds::BinaryReader reader(buffer);
    ds::CompleteBinaryTree<int> tree;
    REQUIRE_THROWS_AS(tree.load(reader), ds::FormatError);
}

TEST_CASE("tree works with doubles", "[tree]") {
    ds::CompleteBinaryTree<double> tree;
    tree.insert(1.5);
    tree.insert(2.5);
    CHECK(tree.toString() == "1.5 2.5");
}
