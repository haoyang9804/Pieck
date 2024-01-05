
file(COPY 
  ${CMAKE_CURRENT_SOURCE_DIR}/codes
  DESTINATION
  ${CMAKE_CURRENT_BINARY_DIR}
)

file(GLOB_RECURSE TESTS
  *.cpp
)

message(STATUS "google test cases include " ${TESTS})

add_executable(
  googletest_${PROJECT_NAME}
  ${TESTS}
)

target_compile_features(googletest_${PROJECT_NAME} PRIVATE ${STD})

target_include_directories(
  googletest_${PROJECT_NAME}
  PRIVATE
  ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(
  googletest_${PROJECT_NAME}
  PRIVATE
  ${PROJECT_NAME}_lib
  GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(googletest_${PROJECT_NAME})

