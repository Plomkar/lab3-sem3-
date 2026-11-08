#define BOOST_TEST_MODULE ds_boost_tests
#include <boost/test/unit_test.hpp>

#include <sstream>
#include <string>
#include <utility>

#include "ds/array.hpp"
#include "ds/errors.hpp"
#include "ds/serialization.hpp"

BOOST_AUTO_TEST_SUITE(ArrayTests)

BOOST_AUTO_TEST_CASE(new_array_is_empty) {
    ds::Array<int> array;
    BOOST_CHECK(array.empty());
    BOOST_CHECK_EQUAL(array.size(), 0U);
    BOOST_CHECK_EQUAL(array.capacity(), 0U);
    BOOST_CHECK_EQUAL(array.kind(), "Array");
    BOOST_CHECK_EQUAL(array.toString(), "");
}

BOOST_AUTO_TEST_CASE(push_back_grows_capacity) {
    ds::Array<int> array;
    for (int i = 0; i < 10; ++i) {
        array.pushBack(i);
    }
    BOOST_CHECK_EQUAL(array.size(), 10U);
    BOOST_CHECK(array.capacity() >= 10U);
    BOOST_CHECK_EQUAL(array.get(0), 0);
    BOOST_CHECK_EQUAL(array.get(9), 9);
    BOOST_CHECK_EQUAL(array.toString(), "0 1 2 3 4 5 6 7 8 9");
}

BOOST_AUTO_TEST_CASE(initializer_list_constructor) {
    const ds::Array<int> array{5, 4, 3, 8, 17};
    BOOST_CHECK_EQUAL(array.size(), 5U);
    BOOST_CHECK_EQUAL(array[3], 8);
    BOOST_CHECK_EQUAL(array.toString(), "5 4 3 8 17");
}

BOOST_AUTO_TEST_CASE(insert_in_middle_front_and_end) {
    ds::Array<int> array{1, 3};
    array.insert(1, 2);
    array.insert(0, 0);
    array.insert(4, 4);
    BOOST_CHECK_EQUAL(array.toString(), "0 1 2 3 4");
}

BOOST_AUTO_TEST_CASE(insert_out_of_range_throws) {
    ds::Array<int> array{1};
    BOOST_CHECK_THROW(array.insert(5, 1), ds::IndexError);
    BOOST_CHECK_EQUAL(array.size(), 1U);
}

BOOST_AUTO_TEST_CASE(get_and_set) {
    ds::Array<int> array{1, 2, 3};
    array.set(1, 20);
    array[2] = 30;
    BOOST_CHECK_EQUAL(array.get(1), 20);
    BOOST_CHECK_EQUAL(array.get(2), 30);
    BOOST_CHECK_THROW(array.get(3), ds::IndexError);
    BOOST_CHECK_THROW(array.set(3, 1), ds::IndexError);
    BOOST_CHECK_THROW(array[7] = 1, ds::IndexError);
    const ds::Array<int>& view = array;
    BOOST_CHECK_THROW(view[7], ds::IndexError);
}

BOOST_AUTO_TEST_CASE(erase_shifts_elements) {
    ds::Array<int> array{5, 4, 3, 8, 17};
    array.erase(2);
    BOOST_CHECK_EQUAL(array.toString(), "5 4 8 17");
    array.erase(3);
    BOOST_CHECK_EQUAL(array.toString(), "5 4 8");
    BOOST_CHECK_THROW(array.erase(3), ds::IndexError);
}

BOOST_AUTO_TEST_CASE(reserve_keeps_content) {
    ds::Array<int> array{1, 2, 3};
    array.reserve(100);
    BOOST_CHECK_EQUAL(array.capacity(), 100U);
    BOOST_CHECK_EQUAL(array.toString(), "1 2 3");
    array.reserve(10);
    BOOST_CHECK_EQUAL(array.capacity(), 100U);
}

BOOST_AUTO_TEST_CASE(clear_keeps_capacity) {
    ds::Array<int> array{1, 2, 3};
    const std::size_t capacity = array.capacity();
    array.clear();
    BOOST_CHECK(array.empty());
    BOOST_CHECK_EQUAL(array.capacity(), capacity);
}

BOOST_AUTO_TEST_CASE(copy_is_deep) {
    ds::Array<int> source{1, 2, 3};
    ds::Array<int> copy(source);
    copy.set(0, 100);
    BOOST_CHECK_EQUAL(source.get(0), 1);
    BOOST_CHECK_EQUAL(copy.get(0), 100);
    ds::Array<int> assigned;
    assigned = source;
    BOOST_CHECK(assigned == source);
    {
        auto& alias = assigned;
        assigned = alias;
    }
    BOOST_CHECK(assigned == source);
}

BOOST_AUTO_TEST_CASE(move_leaves_source_empty) {
    ds::Array<int> source{1, 2, 3};
    ds::Array<int> moved(std::move(source));
    BOOST_CHECK_EQUAL(moved.size(), 3U);
    BOOST_CHECK(source.empty());
    ds::Array<int> target{9};
    target = std::move(moved);
    BOOST_CHECK_EQUAL(target.toString(), "1 2 3");
    BOOST_CHECK(moved.empty());
    {
        auto& alias = target;
        target = std::move(alias);
    }
    BOOST_CHECK_EQUAL(target.size(), 3U);
}

BOOST_AUTO_TEST_CASE(equality_operators) {
    const ds::Array<int> a{1, 2, 3};
    const ds::Array<int> b{1, 2, 3};
    const ds::Array<int> c{1, 2, 4};
    const ds::Array<int> d{1, 2};
    BOOST_CHECK(a == b);
    BOOST_CHECK(a != c);
    BOOST_CHECK(a != d);
}

BOOST_AUTO_TEST_CASE(range_for_iteration) {
    ds::Array<int> array{1, 2, 3, 4};
    int sum = 0;
    for (const int value : array) {
        sum += value;
    }
    BOOST_CHECK_EQUAL(sum, 10);
    for (int& value : array) {
        value *= 2;
    }
    BOOST_CHECK_EQUAL(array.toString(), "2 4 6 8");
}

BOOST_AUTO_TEST_CASE(works_with_double) {
    ds::Array<double> array{1.5, 2.5};
    array.pushBack(3.5);
    BOOST_CHECK_CLOSE(array.get(2), 3.5, 0.0001);
    BOOST_CHECK_EQUAL(array.toString(), "1.5 2.5 3.5");
}

BOOST_AUTO_TEST_CASE(binary_round_trip) {
    const ds::Array<int> source{10, 20, 30, -5};
    std::stringstream buffer;
    ds::BinaryWriter writer(buffer);
    source.save(writer);
    ds::BinaryReader reader(buffer);
    ds::Array<int> loaded{99};
    loaded.load(reader);
    BOOST_CHECK(loaded == source);
}

BOOST_AUTO_TEST_CASE(load_rejects_wrong_tag) {
    std::stringstream buffer;
    ds::BinaryWriter writer(buffer);
    writer.writeTag("XXXX");
    ds::BinaryReader reader(buffer);
    ds::Array<int> array;
    BOOST_CHECK_THROW(array.load(reader), ds::FormatError);
}

BOOST_AUTO_TEST_CASE(load_rejects_truncated_data) {
    std::stringstream buffer;
    ds::BinaryWriter writer(buffer);
    writer.writeTag("ARRY");
    writer.writeU64(3);
    writer.writePod(1);
    ds::BinaryReader reader(buffer);
    ds::Array<int> array;
    BOOST_CHECK_THROW(array.load(reader), ds::FormatError);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(SerializationTests)

BOOST_AUTO_TEST_CASE(u64_round_trip) {
    std::stringstream buffer;
    ds::BinaryWriter writer(buffer);
    writer.writeU64(0);
    writer.writeU64(123456789012345ULL);
    ds::BinaryReader reader(buffer);
    BOOST_CHECK(reader.readU64() == 0ULL);
    BOOST_CHECK(reader.readU64() == 123456789012345ULL);
}

BOOST_AUTO_TEST_CASE(string_round_trip) {
    std::stringstream buffer;
    ds::BinaryWriter writer(buffer);
    writer.writeString("hello");
    writer.writeString("");
    ds::BinaryReader reader(buffer);
    BOOST_CHECK_EQUAL(reader.readString(), "hello");
    BOOST_CHECK_EQUAL(reader.readString(), "");
}

BOOST_AUTO_TEST_CASE(pod_round_trip) {
    std::stringstream buffer;
    ds::BinaryWriter writer(buffer);
    writer.writePod(42);
    writer.writePod(2.5);
    ds::BinaryReader reader(buffer);
    BOOST_CHECK_EQUAL(reader.readPod<int>(), 42);
    BOOST_CHECK_CLOSE(reader.readPod<double>(), 2.5, 0.0001);
}

BOOST_AUTO_TEST_CASE(read_past_end_throws) {
    std::stringstream buffer;
    ds::BinaryReader reader(buffer);
    BOOST_CHECK_THROW(reader.readU64(), ds::FormatError);
}

BOOST_AUTO_TEST_CASE(oversized_count_is_rejected) {
    std::stringstream buffer;
    ds::BinaryWriter writer(buffer);
    writer.writeU64(1ULL << 40U);
    ds::BinaryReader reader(buffer);
    BOOST_CHECK_THROW(reader.readCount(), ds::FormatError);
}

BOOST_AUTO_TEST_CASE(oversized_string_is_rejected) {
    std::stringstream buffer;
    ds::BinaryWriter writer(buffer);
    writer.writeU64(1ULL << 40U);
    ds::BinaryReader reader(buffer);
    BOOST_CHECK_THROW(reader.readString(), ds::FormatError);
}

BOOST_AUTO_TEST_CASE(tag_mismatch_is_rejected) {
    std::stringstream buffer;
    ds::BinaryWriter writer(buffer);
    writer.writeTag("ABCD");
    ds::BinaryReader reader(buffer);
    BOOST_CHECK_THROW(reader.expectTag("WXYZ"), ds::FormatError);
}

BOOST_AUTO_TEST_CASE(tag_match_is_accepted) {
    std::stringstream buffer;
    ds::BinaryWriter writer(buffer);
    writer.writeTag("ABCD");
    ds::BinaryReader reader(buffer);
    BOOST_CHECK_NO_THROW(reader.expectTag("ABCD"));
}

BOOST_AUTO_TEST_CASE(write_to_broken_stream_throws) {
    std::stringstream buffer;
    buffer.setstate(std::ios::badbit);
    ds::BinaryWriter writer(buffer);
    BOOST_CHECK_THROW(writer.writeU64(1), ds::DsError);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(ErrorTests)

BOOST_AUTO_TEST_CASE(error_hierarchy) {
    BOOST_CHECK_THROW(throw ds::IndexError("x"), ds::DsError);
    BOOST_CHECK_THROW(throw ds::EmptyError("x"), ds::DsError);
    BOOST_CHECK_THROW(throw ds::FormatError("x"), ds::DsError);
    BOOST_CHECK_THROW(throw ds::DsError("x"), std::runtime_error);
}

BOOST_AUTO_TEST_SUITE_END()
