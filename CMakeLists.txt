cmake_minimum_required(VERSION 3.16)
project(ChessAIArena)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 17)

set(THREADS_PREFER_PTHREAD_FLAG ON)


add_compile_options(-Wall -Wextra -pedantic -g -lpthread)

add_executable(ChessAIArena main.cpp Chess.cpp Matchmaking.cpp)

find_package(Threads REQUIRED)
target_link_libraries(ChessAIArena PRIVATE Threads::Threads)

