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

add_library(common STATIC
        src/rpc/common.cpp
        )
add_library(my_rpc STATIC
        src/rpc/pci_sniff_clnt.c
        )
link_libraries (pthread common parser my_rpc)
add_executable(server
        src/rpc/server.cpp)
target_include_directories(server
        PUBLIC
        src/parser/include
        )
add_executable(client
        src/rpc/client.cpp)
target_include_directories(client
        PUBLIC
        src/parser/include
        )



