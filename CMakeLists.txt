cmake_minimum_required(VERSION 3.6)
project(CIS457_encrypted_chat)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99 -lcrypto")
include_directories(/usr/include/)
link_directories(openssl)
link_libraries(openssl)

set(SOURCE_FILES chat_server.c tcp_chat.h tcp_chat.c client_list.c client_list.h encrypt.c encrypt.h)
add_executable(CIS457_encrypted_chat ${SOURCE_FILES})