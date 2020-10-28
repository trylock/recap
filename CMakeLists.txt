cmake_minimum_required (VERSION 3.1.0)

project(recap)

set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(EXTERNAL_DIR ${PROJECT_SOURCE_DIR}/external)

set(recap_headers
    ${SRC_DIR}/resistance.hpp
    ${SRC_DIR}/assignment.hpp
)

set(recap_sources
    ${SRC_DIR}/assignment.cpp
)

set(recap_cli ${SRC_DIR}/main.cpp)

# Dependencies
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(Boost 1.71 REQUIRED COMPONENTS program_options)
find_package(TBB 2020 REQUIRED COMPONENTS tbb)
find_package(Threads REQUIRED)

# Compile options
add_compile_options(-Wall -Wextra -pedantic -Wno-stringop-truncation --std=c++17 -g -O3)

# Directories
include_directories(
    ${SRC_DIR}
    ${EXTERNAL_DIR}/fast-cpp-csv-parser
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

# Link
target_link_libraries(recap_cli recap Threads::Threads ${TBB_LIBRARIES} ${TBB_LIBRARIES_RELEASE} ${Boost_LIBRARIES})