#include <chrono>
#include <cstddef>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

#include "ds/array.hpp"
#include "ds/complete_binary_tree.hpp"
#include "ds/database.hpp"
#include "ds/forward_list.hpp"
#include "ds/hash_table.hpp"
#include "ds/linked_list.hpp"
#include "ds/queue.hpp"
#include "ds/stack.hpp"

namespace {

using Clock = std::chrono::steady_clock;

struct Result {
    std::string name;
    std::size_t operations;
    double nanosecondsPerOperation;
};

template <typename T>
void keep(const T& value) {
    asm volatile("" : : "g"(&value) : "memory");
}

Result measure(const std::string& name, std::size_t operations, int repeats, const std::function<void()>& body) {
    double best = -1.0;
    for (int i = 0; i < repeats; ++i) {
        const auto start = Clock::now();
        body();
        const auto stop = Clock::now();
        const double elapsed = std::chrono::duration<double, std::nano>(stop - start).count();
        if (best < 0.0 || elapsed < best) {
            best = elapsed;
        }
    }
    return {name, operations, best / static_cast<double>(operations)};
}

std::vector<std::string> makeKeys(std::size_t count) {
    std::vector<std::string> keys;
    keys.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        keys.push_back("key" + std::to_string(i * 7 + 3));
    }
    return keys;
}

}  // namespace

int main(int argc, char* argv[]) {
    const std::vector<std::string> args(argv + 1, argv + argc);
    const std::size_t n = args.empty() ? 100000 : static_cast<std::size_t>(std::stoul(args[0]));
    const int repeats = 5;
    std::vector<Result> results;

    results.push_back(measure("Array::pushBack", n, repeats, [n] {
        ds::Array<int> array;
        for (std::size_t i = 0; i < n; ++i) {
            array.pushBack(static_cast<int>(i));
        }
        keep(array);
    }));

    results.push_back(measure("Array::get", n, repeats, [n] {
        ds::Array<int> array;
        for (std::size_t i = 0; i < n; ++i) {
            array.pushBack(static_cast<int>(i));
        }
        long long sum = 0;
        for (std::size_t i = 0; i < n; ++i) {
            sum += array.get(i);
        }
        keep(sum);
    }));

    results.push_back(measure("Array::insert(front), 5000 ops", 5000, repeats, [] {
        ds::Array<int> array;
        for (int i = 0; i < 5000; ++i) {
            array.insert(0, i);
        }
        keep(array);
    }));

    results.push_back(measure("ForwardList::pushFront", n, repeats, [n] {
        ds::ForwardList<int> list;
        for (std::size_t i = 0; i < n; ++i) {
            list.pushFront(static_cast<int>(i));
        }
        keep(list);
    }));

    results.push_back(measure("ForwardList::pushBack", n, repeats, [n] {
        ds::ForwardList<int> list;
        for (std::size_t i = 0; i < n; ++i) {
            list.pushBack(static_cast<int>(i));
        }
        keep(list);
    }));

    results.push_back(measure("ForwardList::get(middle), 1000 ops", 1000, repeats, [] {
        ds::ForwardList<int> list;
        for (int i = 0; i < 10000; ++i) {
            list.pushBack(i);
        }
        long long sum = 0;
        for (int i = 0; i < 1000; ++i) {
            sum += list.get(5000);
        }
        keep(sum);
    }));

    results.push_back(measure("LinkedList::pushBack", n, repeats, [n] {
        ds::LinkedList<int> list;
        for (std::size_t i = 0; i < n; ++i) {
            list.pushBack(static_cast<int>(i));
        }
        keep(list);
    }));

    results.push_back(measure("LinkedList::popBack", n, repeats, [n] {
        ds::LinkedList<int> list;
        for (std::size_t i = 0; i < n; ++i) {
            list.pushBack(static_cast<int>(i));
        }
        while (!list.empty()) {
            list.popBack();
        }
        keep(list);
    }));

    results.push_back(measure("Stack::push+pop", n, repeats, [n] {
        ds::Stack<int> stack;
        for (std::size_t i = 0; i < n; ++i) {
            stack.push(static_cast<int>(i));
        }
        while (!stack.empty()) {
            stack.pop();
        }
        keep(stack);
    }));

    results.push_back(measure("Queue::push+pop", n, repeats, [n] {
        ds::Queue<int> queue;
        for (std::size_t i = 0; i < n; ++i) {
            queue.push(static_cast<int>(i));
        }
        while (!queue.empty()) {
            queue.pop();
        }
        keep(queue);
    }));

    results.push_back(measure("CompleteBinaryTree::insert", n, repeats, [n] {
        ds::CompleteBinaryTree<int> tree;
        for (std::size_t i = 0; i < n; ++i) {
            tree.insert(static_cast<int>(i));
        }
        keep(tree);
    }));

    results.push_back(measure("CompleteBinaryTree::inorder", n, repeats, [n] {
        ds::CompleteBinaryTree<int> tree;
        for (std::size_t i = 0; i < n; ++i) {
            tree.insert(static_cast<int>(i));
        }
        const std::vector<int> order = tree.inorder();
        keep(order);
    }));

    const std::vector<std::string> keys = makeKeys(n);
    for (const ds::ProbeStrategy strategy : {ds::ProbeStrategy::Linear, ds::ProbeStrategy::Double}) {
        const std::string label = strategy == ds::ProbeStrategy::Linear ? "linear" : "double";
        results.push_back(measure("HashTable::put (" + label + ")", n, repeats, [&keys, strategy] {
            ds::HashTable<int> table(strategy);
            for (std::size_t i = 0; i < keys.size(); ++i) {
                table.put(keys[i], static_cast<int>(i));
            }
            keep(table);
        }));
        results.push_back(measure("HashTable::get (" + label + ")", n, repeats, [&keys, strategy] {
            ds::HashTable<int> table(strategy);
            for (std::size_t i = 0; i < keys.size(); ++i) {
                table.put(keys[i], static_cast<int>(i));
            }
            const long long sum = std::accumulate(keys.begin(), keys.end(), 0LL, [&table](long long total, const std::string& key) {
                return total + table.get(key).value();
            });
            keep(sum);
        }));
    }

    results.push_back(measure("Database save+load (binary)", n, repeats, [n] {
        ds::Database source;
        for (std::size_t i = 0; i < n / 10; ++i) {
            source.execute("MPUSH " + std::to_string(i));
            source.execute("QPUSH " + std::to_string(i));
        }
        std::stringstream buffer;
        ds::BinaryWriter writer(buffer);
        source.save(writer);
        ds::BinaryReader reader(buffer);
        ds::Database loaded;
        loaded.load(reader);
        keep(loaded);
    }));

    std::cout << std::left << std::setw(40) << "benchmark" << std::right << std::setw(12) << "operations"
              << std::setw(16) << "ns/op" << '\n';
    for (const Result& result : results) {
        std::cout << std::left << std::setw(40) << result.name << std::right << std::setw(12) << result.operations
                  << std::setw(16) << std::fixed << std::setprecision(1) << result.nanosecondsPerOperation << '\n';
    }
    return 0;
}
