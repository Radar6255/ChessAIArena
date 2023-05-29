cmake_minimum_required(VERSION 3.16)
project(ChessAIArena)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 17)

add_compile_options(-Wall -Wextra -pedantic -g)

add_executable(ChessAIArena main.cpp Chess.cpp Matchmaking.cpp)
