cmake_minimum_required(VERSION 3.13.4)

project(improveMTV2 LANGUAGES CXX C)

if(NOT BUILD_TYPE)
    set(BUILD_TYPE origin)
endif()

if(BUILD_TYPE STREQUAL origin)
    set(
        SOURCES
        
        improveMTV2.cpp
        connection.c
        exec_ddta.cpp
        parse.cpp
        resultSet.cpp
        rewriteSQL.cpp
        utilities.cpp
    )

    add_executable(improveMTV2 ${SOURCES})

    find_library(ODBC odbc)
    target_link_libraries(improveMTV2 ${ODBC})

elseif(BUILD_TYPE STREQUAL libpg)
    add_executable(ast AST.cpp)

    target_link_directories(
        ast

        PRIVATE
        vendored/libpg_query
    )
    target_link_libraries(ast pg_query)

    set(
        SOURCES

        libpgMTV2.cpp
        connection.c
        exec_ddta.cpp
        parse.cpp
        resultSet.cpp
        rewriteSQL.cpp
        libpg_utils.cpp        
    )

    add_executable(libpgMTV2 ${SOURCES})   
    
    find_library(ODBC odbc)
    target_link_libraries(libpgMTV2 ${ODBC})

endif()


# 设置 C 编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

# 设置 C++ 编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
