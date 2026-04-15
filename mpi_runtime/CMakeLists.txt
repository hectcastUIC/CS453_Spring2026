cmake_minimum_required(VERSION 3.10)
project(ngs_mpi LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(MPI REQUIRED)

add_executable(ngs_mpi
    src/main.cpp
)

target_link_libraries(ngs_mpi PRIVATE MPI::MPI_CXX)