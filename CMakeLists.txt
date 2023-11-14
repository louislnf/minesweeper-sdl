cmake_minimum_required(VERSION 3.16)

project(minesweeper-sdl)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(
    game
    game.h
    game.cc)

enable_testing()

add_executable(
  game_test
  game_test.cc
)

target_link_libraries(
  game_test
  GTest::gtest_main
  game
)

add_subdirectory(../SDL SDL EXCLUDE_FROM_ALL)

add_executable(
  minesweeper
  main.cc
)

target_link_libraries(minesweeper PRIVATE SDL3::SDL3 game)
target_include_directories(minesweeper PRIVATE "F:/Development/SDL/include")


include(GoogleTest)
gtest_discover_tests(game_test)