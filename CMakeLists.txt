cmake_minimum_required (VERSION 3.16)
project (BufferInsertAlgo)

set (CMAKE_CXX_STANDARD 17)
add_compile_options (-O3 -Werror -Wall)

set (Sources Algo.cpp src/Config.cpp src/RCGraph.cpp)
add_executable (BufferInserter ${Sources})
target_include_directories (BufferInserter PRIVATE include)
