cmake_minimum_required (VERSION 3.8.0)

project(recap CXX CUDA)

set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(EXTERNAL_DIR ${PROJECT_SOURCE_DIR}/external)
set(TEST_DIR ${PROJECT_SOURCE_DIR}/tests)

set(recap_headers
    ${SRC_DIR}/resistance.hpp
    ${SRC_DIR}/assignment.hpp
    ${SRC_DIR}/reassignment.hpp
    ${SRC_DIR}/parallel_assignment_algorithm.hpp
    ${SRC_DIR}/cuda_assignment_algorithm.hpp
)

set(recap_sources
    ${SRC_DIR}/recipe.cpp
    ${SRC_DIR}/parallel_assignment_algorithm.cpp
    ${SRC_DIR}/cuda_assignment_algorithm.cpp
)

set(recap_cuda 
    ${SRC_DIR}/cuda/assignment_kernel.cu
)

set(recap_cli ${SRC_DIR}/main.cpp)

set(recap_tests 
    ${EXTERNAL_DIR}/Catch2/catch_amalgamated.cpp
    ${TEST_DIR}/recipe_test.cpp
    ${TEST_DIR}/assignment_test.cpp
    ${TEST_DIR}/reassignment_test.cpp
)

# CUDA
enable_language("CUDA")

# Dependencies
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(Boost 1.71 REQUIRED COMPONENTS program_options)
find_package(TBB 2020 REQUIRED COMPONENTS tbb)
find_package(Threads REQUIRED)

# Directories
include_directories(
    ${SRC_DIR}
    ${SRC_DIR}/cuda
    ${EXTERNAL_DIR}
    ${EXTERNAL_DIR}/fast-cpp-csv-parser
    ${EXTERNAL_DIR}/Catch2
    ${Boost_INCLUDE_DIRS}
    ${TBB_INCLUDE_DIR}
    /usr/local/cuda/include
)

link_directories(
    ${Boost_LIBRARY_DIRS}
    ${TBB_LIBRARY}
)

# Executables
add_library(recap STATIC ${recap_headers} ${recap_sources} ${recap_cuda})
add_executable(recap_cli ${recap_cli})
add_executable(tests ${recap_tests})

# Compile options
target_compile_options(recap PUBLIC 
    $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -pedantic -fno-omit-frame-pointer --std=c++17 -g>
    $<$<COMPILE_LANGUAGE:CUDA>:--std=c++17 -g>
)

set_target_properties(recap PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

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