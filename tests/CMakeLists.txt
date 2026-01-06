cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

project(bldc_tests C CXX ASM)

find_package(GTest REQUIRED)
include(GoogleTest)

file(GLOB TESTS_SOURCES "${CMAKE_CURRENT_LIST_DIR}/*.c" "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
file(GLOB LIB_SOURCES "${CMAKE_CURRENT_LIST_DIR}/../lib/*.c" "${CMAKE_CURRENT_LIST_DIR}/../lib/*.cpp")
 
add_executable(bldc_tests bldc_tests.cpp ${TESTS_SOURCES} ${LIB_SOURCES})
 
target_link_libraries(bldc_tests GTest::GTest GTest::Main)

target_include_directories(bldc_tests PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../lib
    ${CMAKE_CURRENT_LIST_DIR}/../common
)
 
gtest_discover_tests(bldc_tests)
