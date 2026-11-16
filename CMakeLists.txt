cmake_minimum_required(VERSION 3.21)
project(InterpolationLab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  add_compile_options(/utf-8 /W4)
endif()

# Static library
add_library(InterpolationLib STATIC
  src/Interpolation.cpp
  src/InterpolationTask.cpp
)
target_include_directories(InterpolationLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Demo application
add_executable(InterpolationDemo demo/main.cpp)
target_link_libraries(InterpolationDemo PRIVATE InterpolationLib)

# Misuse of exceptions (int / std::string)
add_executable(AntipatternDemo antipattern/main.cpp)

# Unit tests
find_package(GTest CONFIG REQUIRED)
enable_testing()

add_executable(InterpolationTests tests/InterpolationTests.cpp)
target_link_libraries(InterpolationTests PRIVATE InterpolationLib GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(InterpolationTests)
