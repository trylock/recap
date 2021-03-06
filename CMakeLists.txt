cmake_minimum_required (VERSION 3.8.0)

include(CheckLanguage)

project(recap CXX CUDA)

set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(EXTERNAL_DIR ${PROJECT_SOURCE_DIR}/external)
set(TEST_DIR ${PROJECT_SOURCE_DIR}/tests)

set(recap_headers
    ${SRC_DIR}/recipe.hpp
    ${SRC_DIR}/resistance.hpp
    ${SRC_DIR}/assignment.hpp
    ${SRC_DIR}/equipment.hpp
    ${SRC_DIR}/algorithms/assignment_algorithm.hpp
    ${SRC_DIR}/algorithms/cuda_assignment.hpp
    ${SRC_DIR}/algorithms/parallel_assignment.hpp
)

set(recap_sources
    ${SRC_DIR}/recipe.cpp
    ${SRC_DIR}/algorithms/assignment_algorithm.cpp
    ${SRC_DIR}/algorithms/parallel_assignment.cpp
)

set(recap_cuda 
    ${SRC_DIR}/algorithms/cuda_assignment.cpp
    ${SRC_DIR}/cuda/assignment_kernel.cu
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
find_package(CUDA 10.0)

# Directories
include_directories(
    ${SRC_DIR}
    ${SRC_DIR}/cuda
    ${SRC_DIR}/algorithms
    ${EXTERNAL_DIR}
    ${EXTERNAL_DIR}/fast-cpp-csv-parser
    ${EXTERNAL_DIR}/Catch2
    ${Boost_INCLUDE_DIRS}
    ${TBB_INCLUDE_DIR}
    ${CUDA_INCLUDE_DIRS}
)

link_directories(
    ${Boost_LIBRARY_DIRS}
    ${TBB_LIBRARY}
)

# check CUDA

set(recap_all_sources ${recap_headers} ${recap_sources})
if (CUDA_FOUND)
    add_definitions(-DUSE_CUDA)
    set(recap_all_sources ${recap_headers} ${recap_sources} ${recap_cuda})    
endif()

# Executables
add_library(recap STATIC ${recap_all_sources})
add_executable(recap_cli ${recap_cli})
add_executable(tests ${recap_tests})

# Compile options
target_compile_options(recap PUBLIC 
    $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -pedantic -fno-omit-frame-pointer --std=c++17 -g -O3>
    $<$<COMPILE_LANGUAGE:CUDA>:--std=c++17 -g -O3 -arch compute_52>
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