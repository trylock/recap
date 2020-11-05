cmake_minimum_required (VERSION 3.1.0)

project(recap)

set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(EXTERNAL_DIR ${PROJECT_SOURCE_DIR}/external)
set(TEST_DIR ${PROJECT_SOURCE_DIR}/tests)

set(recap_headers
    ${SRC_DIR}/resistance.hpp
    ${SRC_DIR}/assignment.hpp
    ${SRC_DIR}/reassignment.hpp
)

set(recap_sources
    ${SRC_DIR}/recipe.cpp
    ${SRC_DIR}/assignment.cpp
    ${SRC_DIR}/reassignment.cpp
)

set(recap_cli ${SRC_DIR}/main.cpp)

set(recap_tests 
    ${EXTERNAL_DIR}/Catch2/catch_amalgamated.cpp
    ${TEST_DIR}/recipe_test.cpp
    ${TEST_DIR}/assignment_test.cpp
    ${TEST_DIR}/reassignment_test.cpp
)

# Dependencies
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(Boost 1.71 REQUIRED COMPONENTS program_options)
find_package(TBB 2020 REQUIRED COMPONENTS tbb)
find_package(Threads REQUIRED)

# Compile options
add_compile_options(-Wall -Wextra -pedantic -fno-omit-frame-pointer --std=c++17 -g -O3)

# Directories
include_directories(
    ${SRC_DIR}
    ${EXTERNAL_DIR}
    ${EXTERNAL_DIR}/fast-cpp-csv-parser
    ${EXTERNAL_DIR}/Catch2
    ${Boost_INCLUDE_DIRS}
    ${TBB_INCLUDE_DIR}
)

link_directories(
    ${Boost_LIBRARY_DIRS}
    ${TBB_LIBRARY}
)

# Executables
add_library(recap ${recap_headers} ${recap_sources})
add_executable(recap_cli ${recap_cli})
add_executable(tests ${recap_tests})

# Link
target_link_libraries(recap_cli 
    recap 
    Threads::Threads 
    ${TBB_LIBRARIES} 
    ${TBB_LIBRARIES_RELEASE} 
    ${Boost_LIBRARIES})

target_link_libraries(tests
    recap 
    Threads::Threads 
    ${TBB_LIBRARIES} 
    ${TBB_LIBRARIES_RELEASE} 
    ${Boost_LIBRARIES})