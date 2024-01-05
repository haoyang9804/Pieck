cmake_minimum_required(VERSION 3.14)

project(Pieck)

# Debug
option(DEBUG "Enable Debugging." ON)
if(DEBUG)
  message(STATUS "Debug Mode On")
  add_definitions(-DDEBUG)
endif()

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Clang REQUIRED)
find_program(CLANG_TIDY_EXE NAMES "clang-tidy" REQUIRED)
find_program(CLANG_FORMAT_EXE NAMES "clang-format" REQUIRED)
set(CLANG_TIDY_COMMAND "${CLANG_TIDY_EXE}" "-checks=-*,clang-analyzer-*")
set(CLANG_FORMAT_COMMAND "${CLANG_FORMAT_EXE}")

message(STATUS "ClangConfig.cmake: ${Clang_DIR}/ClangConfig.cmake")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
include(AddLLVM)
include(${Clang_DIR}/AddClang.cmake)


file(GLOB_RECURSE SRCS 
    src/*.cpp
)
add_library(${PROJECT_NAME}_lib SHARED ${SRCS})
target_include_directories(${PROJECT_NAME}_lib
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)
if(DEBUG)
  set_target_properties(${PROJECT_NAME}_lib PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
endif()

target_link_libraries(${PROJECT_NAME}_lib
  PRIVATE
  clangAST
  clangBasic
  clangFrontend
  clangSerialization
  clangTooling
)

file(GLOB_RECURSE ALL_CXX_SOURCE_FILES
  ${PROJECT_SOURCE_DIR}/*.cpp
  ${PROJECT_SOURCE_DIR}/*.h
)

add_custom_target(
    format 
    COMMAND ${CLANG_FORMAT_COMMAND}
    -i
    -style=file
    ${ALL_CXX_SOURCE_FILES}
)

if(DEBUG)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  )
  # For Windows: Prevent overriding the parent project's compiler/linker settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  enable_testing()

  add_subdirectory(unittests)
endif()