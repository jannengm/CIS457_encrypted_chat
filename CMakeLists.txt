cmake_minimum_required(VERSION 3.6)
project(CIS457_encrypted_chat)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99")

set(SOURCE_FILES chat_server.c chat_client.c)
add_executable(CIS457_encrypted_chat ${SOURCE_FILES})