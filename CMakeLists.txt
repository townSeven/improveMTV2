cmake_minimum_required(VERSION 3.13.4)

project(improveMTV2 LANGUAGES CXX C)

add_executable(
    improveMTV2 

    improveMTV2.cpp
    connection.c
    exec_ddta.cpp
    parse.cpp
    resultSet.cpp
    rewriteSQL.cpp
    utilities.cpp
)

find_library(ODBC odbc)

target_link_libraries(
    improveMTV2

    ${ODBC}
)

# 设置 C 编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

# 设置 C++ 编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
