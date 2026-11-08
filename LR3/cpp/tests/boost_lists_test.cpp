#include <boost/test/unit_test.hpp>

#include <sstream>
#include <utility>
#include <vector>

#include "ds/errors.hpp"
#include "ds/forward_list.hpp"
#include "ds/linked_list.hpp"

BOOST_AUTO_TEST_SUITE(ForwardListTests)

BOOST_AUTO_TEST_CASE(new_list_is_empty) {
    ds::ForwardList<int> list;
    BOOST_CHECK(list.empty());
    BOOST_CHECK_EQUAL(list.kind(), "ForwardList");
    BOOST_CHECK_THROW(list.front(), ds::EmptyError);
    BOOST_CHECK_THROW(list.back(), ds::EmptyError);
    BOOST_CHECK_THROW(list.popFront(), ds::EmptyError);
}

BOOST_AUTO_TEST_CASE(push_front_and_back) {
    ds::ForwardList<int> list;
    list.pushFront(2);
    list.pushFront(1);
    list.pushBack(3);
    BOOST_CHECK_EQUAL(list.toString(), "1 2 3");
    BOOST_CHECK_EQUAL(list.front(), 1);
    BOOST_CHECK_EQUAL(list.back(), 3);
}

BOOST_AUTO_TEST_CASE(insert_at_positions) {
    ds::ForwardList<int> list{1, 4};
    list.insert(1, 2);
    list.insert(2, 3);
    list.insert(0, 0);
    list.insert(5, 5);
    BOOST_CHECK_EQUAL(list.toString(), "0 1 2 3 4 5");
    BOOST_CHECK_THROW(list.insert(10, 1), ds::IndexError);
}

BOOST_AUTO_TEST_CASE(insert_after_index) {
    ds::ForwardList<int> list{1, 3};
    list.insertAfter(0, 2);
    list.insertAfter(2, 4);
    BOOST_CHECK_EQUAL(list.toString(), "1 2 3 4");
    BOOST_CHECK_EQUAL(list.back(), 4);
    BOOST_CHECK_THROW(list.insertAfter(4, 1), ds::IndexError);
}

BOOST_AUTO_TEST_CASE(pop_front_returns_values) {
    ds::ForwardList<int> list{1, 2};
    BOOST_CHECK_EQUAL(list.popFront(), 1);
    BOOST_CHECK_EQUAL(list.popFront(), 2);
    BOOST_CHECK(list.empty());
    list.pushBack(7);
    BOOST_CHECK_EQUAL(list.back(), 7);
}

BOOST_AUTO_TEST_CASE(erase_at_head_middle_tail) {
    ds::ForwardList<int> list{1, 2, 3, 4, 5};
    list.eraseAt(0);
    list.eraseAt(1);
    list.eraseAt(2);
    BOOST_CHECK_EQUAL(list.toString(), "2 4");
    BOOST_CHECK_EQUAL(list.back(), 4);
    BOOST_CHECK_THROW(list.eraseAt(2), ds::IndexError);
    list.pushBack(6);
    BOOST_CHECK_EQUAL(list.toString(), "2 4 6");
}

BOOST_AUTO_TEST_CASE(erase_by_value) {
    ds::ForwardList<int> list{1, 2, 3, 2};
    BOOST_CHECK(list.eraseValue(2));
    BOOST_CHECK_EQUAL(list.toString(), "1 3 2");
    BOOST_CHECK(list.eraseValue(1));
    BOOST_CHECK(list.eraseValue(2));
    BOOST_CHECK_EQUAL(list.back(), 3);
    BOOST_CHECK(!list.eraseValue(42));
    BOOST_CHECK(list.eraseValue(3));
    BOOST_CHECK(list.empty());
}

BOOST_AUTO_TEST_CASE(get_and_index_of) {
    const ds::ForwardList<int> list{10, 20, 30};
    BOOST_CHECK_EQUAL(list.get(1), 20);
    BOOST_CHECK_THROW(list.get(3), ds::IndexError);
    const auto found = list.indexOf(30);
    BOOST_CHECK(found.has_value());
    BOOST_CHECK_EQUAL(*found, 2U);
    BOOST_CHECK(!list.indexOf(99).has_value());
}

BOOST_AUTO_TEST_CASE(to_vector_matches_content) {
    const ds::ForwardList<int> list{1, 2, 3};
    const std::vector<int> expected{1, 2, 3};
    BOOST_CHECK(list.toVector() == expected);
}

BOOST_AUTO_TEST_CASE(copy_and_move) {
    ds::ForwardList<int> source{1, 2, 3};
    ds::ForwardList<int> copy(source);
    copy.pushBack(4);
    BOOST_CHECK_EQUAL(source.size(), 3U);
    BOOST_CHECK_EQUAL(copy.size(), 4U);
    ds::ForwardList<int> assigned;
    assigned = source;
    {
        auto& alias = assigned;
        assigned = alias;
    }
    BOOST_CHECK_EQUAL(assigned.toString(), "1 2 3");
    ds::ForwardList<int> moved(std::move(source));
    BOOST_CHECK(source.empty());
    BOOST_CHECK_EQUAL(moved.toString(), "1 2 3");
    ds::ForwardList<int> target{9};
    target = std::move(moved);
    BOOST_CHECK_EQUAL(target.toString(), "1 2 3");
    {
        auto& alias = target;
        target = std::move(alias);
    }
    BOOST_CHECK_EQUAL(target.size(), 3U);
}

BOOST_AUTO_TEST_CASE(clear_empties_list) {
    ds::ForwardList<int> list{1, 2, 3};
    list.clear();
    BOOST_CHECK(list.empty());
    BOOST_CHECK_EQUAL(list.toString(), "");
}

BOOST_AUTO_TEST_CASE(binary_round_trip) {
    const ds::ForwardList<int> source{7, 8, 9};
    std::stringstream buffer;
    ds::BinaryWriter writer(buffer);
    source.save(writer);
    ds::BinaryReader reader(buffer);
    ds::ForwardList<int> loaded{1};
    loaded.load(reader);
    BOOST_CHECK_EQUAL(loaded.toString(), "7 8 9");
}

BOOST_AUTO_TEST_CASE(load_rejects_wrong_tag) {
    std::stringstream buffer;
    ds::BinaryWriter writer(buffer);
    writer.writeTag("NOPE");
    ds::BinaryReader reader(buffer);
    ds::ForwardList<int> list;
    BOOST_CHECK_THROW(list.load(reader), ds::FormatError);
}

BOOST_AUTO_TEST_CASE(long_list_is_destroyed_without_recursion) {
    ds::ForwardList<int> list;
    for (int i = 0; i < 200000; ++i) {
        list.pushBack(i);
    }
    BOOST_CHECK_EQUAL(list.size(), 200000U);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(LinkedListTests)

BOOST_AUTO_TEST_CASE(new_list_is_empty) {
    ds::LinkedList<int> list;
    BOOST_CHECK(list.empty());
    BOOST_CHECK_EQUAL(list.kind(), "LinkedList");
    BOOST_CHECK_THROW(list.front(), ds::EmptyError);
    BOOST_CHECK_THROW(list.back(), ds::EmptyError);
    BOOST_CHECK_THROW(list.popFront(), ds::EmptyError);
    BOOST_CHECK_THROW(list.popBack(), ds::EmptyError);
}

BOOST_AUTO_TEST_CASE(push_front_and_back) {
    ds::LinkedList<int> list;
    list.pushBack(2);
    list.pushFront(1);
    list.pushBack(3);
    BOOST_CHECK_EQUAL(list.toString(), "1 2 3");
    BOOST_CHECK_EQUAL(list.toReversedString(), "3 2 1");
    BOOST_CHECK_EQUAL(list.front(), 1);
    BOOST_CHECK_EQUAL(list.back(), 3);
}

BOOST_AUTO_TEST_CASE(insert_at_positions) {
    ds::LinkedList<int> list{1, 4};
    list.insert(1, 2);
    list.insert(2, 3);
    list.insert(0, 0);
    list.insert(5, 5);
    BOOST_CHECK_EQUAL(list.toString(), "0 1 2 3 4 5");
    BOOST_CHECK_EQUAL(list.toReversedString(), "5 4 3 2 1 0");
    BOOST_CHECK_THROW(list.insert(10, 1), ds::IndexError);
}

BOOST_AUTO_TEST_CASE(insert_after_index) {
    ds::LinkedList<int> list{1, 3};
    list.insertAfter(0, 2);
    list.insertAfter(2, 4);
    BOOST_CHECK_EQUAL(list.toString(), "1 2 3 4");
    BOOST_CHECK_EQUAL(list.toReversedString(), "4 3 2 1");
    BOOST_CHECK_THROW(list.insertAfter(4, 1), ds::IndexError);
}

BOOST_AUTO_TEST_CASE(pop_front_and_back) {
    ds::LinkedList<int> list{1, 2, 3};
    BOOST_CHECK_EQUAL(list.popFront(), 1);
    BOOST_CHECK_EQUAL(list.popBack(), 3);
    BOOST_CHECK_EQUAL(list.popBack(), 2);
    BOOST_CHECK(list.empty());
    list.pushFront(5);
    BOOST_CHECK_EQUAL(list.back(), 5);
}

BOOST_AUTO_TEST_CASE(erase_at_head_middle_tail) {
    ds::LinkedList<int> list{1, 2, 3, 4, 5};
    list.eraseAt(0);
    list.eraseAt(3);
    list.eraseAt(1);
    BOOST_CHECK_EQUAL(list.toString(), "2 4");
    BOOST_CHECK_EQUAL(list.toReversedString(), "4 2");
    BOOST_CHECK_THROW(list.eraseAt(2), ds::IndexError);
}

BOOST_AUTO_TEST_CASE(erase_by_value) {
    ds::LinkedList<int> list{1, 2, 3, 2};
    BOOST_CHECK(list.eraseValue(2));
    BOOST_CHECK_EQUAL(list.toString(), "1 3 2");
    BOOST_CHECK(!list.eraseValue(42));
    BOOST_CHECK(list.eraseValue(2));
    BOOST_CHECK_EQUAL(list.back(), 3);
}

BOOST_AUTO_TEST_CASE(get_from_both_halves) {
    const ds::LinkedList<int> list{0, 1, 2, 3, 4, 5, 6, 7};
    BOOST_CHECK_EQUAL(list.get(0), 0);
    BOOST_CHECK_EQUAL(list.get(2), 2);
    BOOST_CHECK_EQUAL(list.get(5), 5);
    BOOST_CHECK_EQUAL(list.get(7), 7);
    BOOST_CHECK_THROW(list.get(8), ds::IndexError);
}

BOOST_AUTO_TEST_CASE(index_of) {
    const ds::LinkedList<int> list{10, 20, 30};
    const auto found = list.indexOf(20);
    BOOST_CHECK(found.has_value());
    BOOST_CHECK_EQUAL(*found, 1U);
    BOOST_CHECK(!list.indexOf(99).has_value());
}

BOOST_AUTO_TEST_CASE(copy_and_move) {
    ds::LinkedList<int> source{1, 2, 3};
    ds::LinkedList<int> copy(source);
    copy.pushFront(0);
    BOOST_CHECK_EQUAL(source.size(), 3U);
    BOOST_CHECK_EQUAL(copy.toString(), "0 1 2 3");
    ds::LinkedList<int> assigned;
    assigned = source;
    {
        auto& alias = assigned;
        assigned = alias;
    }
    BOOST_CHECK_EQUAL(assigned.toString(), "1 2 3");
    ds::LinkedList<int> moved(std::move(source));
    BOOST_CHECK(source.empty());
    BOOST_CHECK_EQUAL(moved.toString(), "1 2 3");
    ds::LinkedList<int> target{9};
    target = std::move(moved);
    BOOST_CHECK_EQUAL(target.toString(), "1 2 3");
    {
        auto& alias = target;
        target = std::move(alias);
    }
    BOOST_CHECK_EQUAL(target.size(), 3U);
}

BOOST_AUTO_TEST_CASE(clear_empties_list) {
    ds::LinkedList<int> list{1, 2, 3};
    list.clear();
    BOOST_CHECK(list.empty());
    BOOST_CHECK_EQUAL(list.toReversedString(), "");
}

BOOST_AUTO_TEST_CASE(binary_round_trip) {
    const ds::LinkedList<int> source{7, 8, 9};
    std::stringstream buffer;
    ds::BinaryWriter writer(buffer);
    source.save(writer);
    ds::BinaryReader reader(buffer);
    ds::LinkedList<int> loaded{1};
    loaded.load(reader);
    BOOST_CHECK_EQUAL(loaded.toString(), "7 8 9");
    BOOST_CHECK_EQUAL(loaded.toReversedString(), "9 8 7");
}

BOOST_AUTO_TEST_CASE(load_rejects_wrong_tag) {
    std::stringstream buffer;
    ds::BinaryWriter writer(buffer);
    writer.writeTag("NOPE");
    ds::BinaryReader reader(buffer);
    ds::LinkedList<int> list;
    BOOST_CHECK_THROW(list.load(reader), ds::FormatError);
}

BOOST_AUTO_TEST_SUITE_END()
