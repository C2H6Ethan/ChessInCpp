cmake_minimum_required(VERSION 3.30)
project(ChessInCpp)

set(CMAKE_CXX_STANDARD 26)

add_executable(ChessInCpp main.cpp
        Board.cpp
        Board.h
        BitboardUtils.h
        Move.h)
