find_package(GTest REQUIRED)
find_package(Boost REQUIRED COMPONENTS unit_test_framework)
find_package(Catch2 3 REQUIRED)

add_executable(test_boost
    boost_array_test.cpp
    boost_lists_test.cpp)
target_compile_definitions(test_boost PRIVATE BOOST_TEST_DYN_LINK)
target_link_libraries(test_boost PRIVATE ds Boost::unit_test_framework)
add_test(NAME boost_array_and_lists COMMAND test_boost)

add_executable(test_gtest
    gtest_stack_queue_test.cpp
    gtest_hash_test.cpp)
target_link_libraries(test_gtest PRIVATE ds GTest::gtest_main)
add_test(NAME gtest_stack_queue_hash COMMAND test_gtest)

add_executable(test_catch2
    catch2_tree_test.cpp
    catch2_database_test.cpp)
target_link_libraries(test_catch2 PRIVATE ds Catch2::Catch2WithMain)
add_test(NAME catch2_tree_and_database COMMAND test_catch2)
