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
  set (COMPILE_OPTIONS "/W4;/WX")
  set (RELEASE_COMPILE_OPTIONS "${COMPILE_OPTIONS};/O2")
  target_compile_options(${PROJECT_NAME} PRIVATE "$<$<CONFIG:RELEASE>:${RELEASE_COMPILE_OPTIONS}>")
  target_compile_options(${PROJECT_NAME} PRIVATE "$<$<CONFIG:DEBUG>:${DEBUG_COMPILE_OPTIONS}>")
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -O3 -Wall -Wextra -Wpedantic)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE "DEBUG=$<IF:$<CONFIG:Debug>,1,0>")

target_include_directories (${PROJECT_NAME} PRIVATE include)
