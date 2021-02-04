#CMake 最低版本号
cmake_minimum_required(VERSION 3.11.2)
project(ssxrver)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g -lboost_thread -Wall -Wconversion -Wredundant-decls -Wcast-align -Wshadow -Werror -Wold-style-cast -march=native -pthread -Wno-unused-variable -fopenmp -L /usr/lib64/mysql -lmysqlclient -I  /usr/include/mysql")
elseif( CMAKE_BUILD_TYPE STREQUAL "Release" )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -o3 -lboost_thread -Wall -Wconversion -Wredundant-decls -Wcast-align -Wshadow -Werror -Wold-style-cast -march=native -pthread -Wno-unused-variable -fopenmp -L /usr/lib64/mysql -lmysqlclient -I  /usr/include/mysql")
endif()

message("Build Type:" ${CMAKE_BUILD_TYPE} ${CMAKE_CXX_FLAGS})


include_directories(base)
include_directories(http)
include_directories(net)
include_directories(util)

add_executable(ssxrver
        base/AsyncLogThread.cpp
        base/AsyncLogThread.h
        base/CurrentThread.h
        base/Exception.cpp
        base/Exception.h
        base/File.cpp
        base/File.h
        base/LogFile.cpp
        base/LogFile.h
        base/Logging.cpp
        base/Logging.h
        base/LogStream.cpp
        base/LogStream.h
        base/Thread.cpp
        base/Thread.h
        http/http11_common.h
        http/http11_parser.cpp
        http/http11_parser.h
        http/httpclient_parser.cpp
        http/httpclient_parser.h
        http/HttpParser.cpp
        http/HttpParser.h
        http/HttpRequest.h
        http/HttpResponse.cpp
        http/HttpResponse.h
        http/HttpServer.cpp
        http/HttpServer.h
        net/Buffer.cpp
        net/Buffer.h
        net/CallBacks.h
        net/Channel.cpp
        net/Channel.h
        util/cJSON.c
        util/cJSON.h
        util/CJsonObject.cpp
        util/CJsonObject.hpp
        net/Epoll.cpp
        net/Epoll.h
        net/EventLoop.cpp
        net/EventLoop.h
        net/EventLoopThread.cpp
        net/EventLoopThread.h
        net/EventLoopThreadPool.cpp
        net/EventLoopThreadPool.h
        util/MySQL.cpp
        util/MySQL.h
        util/MySQLOps.cpp
        util/MySQLOps.h
        net/SocketOps.cpp
        net/SocketOps.h
        net/TcpConnection.cpp
        net/TcpConnection.h
        net/TcpServer.cpp
        net/TcpServer.h
        util/Init.cpp
        util/Init.h
        main.cpp)
