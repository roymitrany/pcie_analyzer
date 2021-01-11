cmake_minimum_required(VERSION 3.11)
project(rdma_pcie_parser)

set(CMAKE_CXX_STANDARD 11)

add_library(parser
        src/parser/src/LogicChunk.cpp
        src/parser/src/LogicPacketsStream.cpp
        src/parser/src/structs.cpp
        src/parser/src/token.cpp
        src/parser/src/VovitChunk.cpp
        )

target_include_directories(parser
        PUBLIC
        src/parser/include
        )


link_libraries (parser)
add_executable(server
        src/rpc/server.cpp)
target_include_directories(server
        PUBLIC
        src/parser/include
        )




