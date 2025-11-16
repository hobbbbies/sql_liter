cmake_minimum_required(VERSION 3.14)
project(sql_liter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# Library sources (excluding main.cpp for testing)
set(LIB_SOURCES
    src/tokenizer.cpp
    src/row.cpp
    src/table.cpp
    src/input_buffer.cpp
    src/statement_processor.cpp
    src/meta_command_processor.cpp
    src/utils.cpp
    src/pager.cpp
    src/cursor.cpp
    src/node.cpp
)

# Create a library for the core functionality
add_library(sql_liter_lib ${LIB_SOURCES})

# Main executable
add_executable(sql_liter src/main.cpp)
target_link_libraries(sql_liter sql_liter_lib)

# Compiler warnings
if (MSVC)
  target_compile_options(sql_liter_lib PRIVATE /W4)
  target_compile_options(sql_liter PRIVATE /W4)
else()
  target_compile_options(sql_liter_lib PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(sql_liter PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()

# Test sources
set(TEST_SOURCES
    tests/test_row.cpp
    tests/test_table.cpp
    tests/test_tokenizer.cpp
    tests/test_input_integration.cpp
    tests/test_pager.cpp
    tests/test_cursor.cpp
    tests/test_node.cpp
)

# Test executable
add_executable(sql_liter_tests ${TEST_SOURCES})
target_link_libraries(sql_liter_tests 
    sql_liter_lib
    gtest_main
    gtest
)

# Discover tests
include(GoogleTest)
gtest_discover_tests(sql_liter_tests)
