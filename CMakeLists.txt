cmake_minimum_required (VERSION 3.12)

project(glyph)

include(CheckCCompilerFlag)
check_c_compiler_flag("-Wconversion" has_warn_conversion "int main() { return 0; }")
if (has_warn_conversion)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wconversion")
endif()

include_directories(include)
file(GLOB TEST_ALL_SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "tests/*.c")
foreach(TEST_SOURCE ${TEST_ALL_SOURCES})
  string(REGEX REPLACE "^.*/([^/]*)\\.[^.]*$" "test_\\1" TEST_NAME ${TEST_SOURCE})
  add_executable(${TEST_NAME} ${TEST_SOURCE})
endforeach()
