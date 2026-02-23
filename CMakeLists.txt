cmake_minimum_required(VERSION 3.30)
project(ChessInCpp)

set(CMAKE_CXX_STANDARD 26)

# 1. Core library containing ALL domain logic and the validator
add_library(ChessCore
        Board.cpp
        Board.h
        BitboardUtils.h
        Move.h
        Validator.cpp
        Validator.h
)

# 2. Production CLI binary
add_executable(validator main.cpp)
target_link_libraries(validator PRIVATE ChessCore)

# 3. Catch2 Test binary
add_executable(run_tests test_validator.cpp catch_amalgamated.cpp)
target_link_libraries(run_tests PRIVATE ChessCore)