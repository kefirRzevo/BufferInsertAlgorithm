cmake_minimum_required (VERSION 3.16)
project (BufferInsertAlgo)

set (CMAKE_CXX_STANDARD 20)
set (Sources
  Algo.cpp
  src/Config.cpp
  src/RCGraph.cpp
  src/SolutionInsertion.cpp
  src/BufferAlgorithm.cpp
)
add_executable (BufferInserter ${Sources})

if(MSVC)
  target_compile_options(BufferInserter PRIVATE -O3 /W4 /WX)
else()
  target_compile_options(BufferInserter PRIVATE -O3 -Wall -Wextra -Wpedantic -Werror)
endif()

target_include_directories (BufferInserter PRIVATE include)
