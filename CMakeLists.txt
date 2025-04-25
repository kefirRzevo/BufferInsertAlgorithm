cmake_minimum_required (VERSION 3.16)
project(BufferInserter) 

set (CMAKE_CXX_STANDARD 20)
set (Sources
  Algo.cpp
  src/Config.cpp
  src/RCGraph.cpp
  src/SolutionInsertion.cpp
  src/BufferAlgorithm.cpp
)
add_executable (${PROJECT_NAME} ${Sources})

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE -O3 /W4 /WX)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -O3 -Wall -Wextra -Wpedantic)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE "DEBUG=$<IF:$<CONFIG:Debug>,1,0>")

target_include_directories (${PROJECT_NAME} PRIVATE include)
