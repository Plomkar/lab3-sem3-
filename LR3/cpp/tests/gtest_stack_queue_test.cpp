#include <gtest/gtest.h>

#include <sstream>
#include <utility>

#include "ds/errors.hpp"
#include "ds/queue.hpp"
#include "ds/stack.hpp"

class StackTest : public ::testing::Test {
protected:
    void SetUp() override {
        stack.push(1);
        stack.push(2);
        stack.push(3);
    }

    ds::Stack<int> stack;
};

TEST(StackBasics, NewStackIsEmpty) {
    ds::Stack<int> stack;
    EXPECT_TRUE(stack.empty());
    EXPECT_EQ(stack.size(), 0U);
    EXPECT_EQ(stack.kind(), "Stack");
    EXPECT_THROW(stack.pop(), ds::EmptyError);
    EXPECT_THROW(stack.peek(), ds::EmptyError);
}

TEST(StackBasics, InitializerListPushesInOrder) {
    ds::Stack<int> stack{1, 2, 3};
    EXPECT_EQ(stack.peek(), 3);
    EXPECT_EQ(stack.toString(), "3 2 1");
}

TEST_F(StackTest, PopReturnsLastPushed) {
    EXPECT_EQ(stack.pop(), 3);
    EXPECT_EQ(stack.pop(), 2);
    EXPECT_EQ(stack.pop(), 1);
    EXPECT_TRUE(stack.empty());
}

TEST_F(StackTest, PeekDoesNotRemove) {
    EXPECT_EQ(stack.peek(), 3);
    EXPECT_EQ(stack.size(), 3U);
}

TEST_F(StackTest, ClearEmptiesStack) {
    stack.clear();
    EXPECT_TRUE(stack.empty());
    EXPECT_EQ(stack.toString(), "");
}

TEST_F(StackTest, BinaryRoundTrip) {
    std::stringstream buffer;
    ds::BinaryWriter writer(buffer);
    stack.save(writer);
    ds::BinaryReader reader(buffer);
    ds::Stack<int> loaded;
    loaded.push(100);
    loaded.load(reader);
    EXPECT_EQ(loaded.size(), 3U);
    EXPECT_EQ(loaded.pop(), 3);
    EXPECT_EQ(loaded.pop(), 2);
    EXPECT_EQ(loaded.pop(), 1);
}

TEST_F(StackTest, LoadRejectsWrongTag) {
    std::stringstream buffer;
    ds::BinaryWriter writer(buffer);
    writer.writeTag("NOPE");
    ds::BinaryReader reader(buffer);
    EXPECT_THROW(stack.load(reader), ds::FormatError);
}

TEST_F(StackTest, CopyIsIndependent) {
    ds::Stack<int> copy = stack;
    copy.push(4);
    EXPECT_EQ(stack.size(), 3U);
    EXPECT_EQ(copy.size(), 4U);
    ds::Stack<int> moved = std::move(copy);
    EXPECT_EQ(moved.peek(), 4);
}

TEST(StackPolymorphism, UsedThroughContainerPointer) {
    ds::Stack<int> stack{1, 2};
    const ds::Container* container = &stack;
    EXPECT_EQ(container->size(), 2U);
    EXPECT_EQ(container->kind(), "Stack");
    EXPECT_FALSE(container->empty());
}

class QueueTest : public ::testing::Test {
protected:
    void SetUp() override {
        queue.push(1);
        queue.push(2);
        queue.push(3);
    }

    ds::Queue<int> queue;
};

TEST(QueueBasics, NewQueueIsEmpty) {
    ds::Queue<int> queue;
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.kind(), "Queue");
    EXPECT_THROW(queue.pop(), ds::EmptyError);
    EXPECT_THROW(queue.peek(), ds::EmptyError);
}

TEST(QueueBasics, InitializerListKeepsOrder) {
    ds::Queue<int> queue{1, 2, 3};
    EXPECT_EQ(queue.peek(), 1);
    EXPECT_EQ(queue.toString(), "1 2 3");
}

TEST_F(QueueTest, PopFollowsFifoOrder) {
    EXPECT_EQ(queue.pop(), 1);
    EXPECT_EQ(queue.pop(), 2);
    EXPECT_EQ(queue.pop(), 3);
    EXPECT_TRUE(queue.empty());
}

TEST_F(QueueTest, PeekDoesNotRemove) {
    EXPECT_EQ(queue.peek(), 1);
    EXPECT_EQ(queue.size(), 3U);
}

TEST_F(QueueTest, ClearEmptiesQueue) {
    queue.clear();
    EXPECT_TRUE(queue.empty());
}

TEST_F(QueueTest, BinaryRoundTrip) {
    std::stringstream buffer;
    ds::BinaryWriter writer(buffer);
    queue.save(writer);
    ds::BinaryReader reader(buffer);
    ds::Queue<int> loaded;
    loaded.push(100);
    loaded.load(reader);
    EXPECT_EQ(loaded.toString(), "1 2 3");
}

TEST_F(QueueTest, LoadRejectsWrongTag) {
    std::stringstream buffer;
    ds::BinaryWriter writer(buffer);
    writer.writeTag("NOPE");
    ds::BinaryReader reader(buffer);
    EXPECT_THROW(queue.load(reader), ds::FormatError);
}

TEST_F(QueueTest, ReuseAfterDrain) {
    while (!queue.empty()) {
        queue.pop();
    }
    queue.push(9);
    EXPECT_EQ(queue.peek(), 9);
}
