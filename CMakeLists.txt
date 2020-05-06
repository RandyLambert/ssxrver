#CMake 最低版本好要求
cmake_minimum_required (VERSION 3.11.2)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -I/usr/include/mysql -g -Wall -Wno-unused-variable -Wshadow -Werror -Wold-style-cast -L/usr/lib/mysql -lmysqlclient -pthread")
elseif( CMAKE_BUILD_TYPE STREQUAL "Release" )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -I/usr/include/mysql -O2 -Wall -Wshadow -Werror -Wold-style-cast -L/usr/lib/mysql -lmysqlclient -pthread -fopenmp")
endif()

message("Build Type:" ${CMAKE_BUILD_TYPE} ${CMAKE_CXX_FLAGS})


# set(CXX_FLAGS
#  -g
#  # -DVALGRIND
#  -DCHECK_PTHREAD_RETURN_VALUE
#  -D_FILE_OFFSET_BITS=64
#  -Wall
#  -Wextra
#  -Werror
#  -Wconversion
#  -Wno-unused-parameter
#  -Wold-style-cast
#  -Woverloaded-virtual
#  -Wpointer-arith
#  -Wshadow
#  -Wwrite-strings
#  -march=native
#  # -MMD
#  -lstd=c++17
#  -rdynamic
#  )

#项目信息
project(web_ssxrver)

# 查找当前目录下的所有源文件
# 并将名称保存到DIR——SRCS变量
aux_source_directory(. DIR_SRCS)

# 添加net http base子目录
add_subdirectory(base)
add_subdirectory(net)
add_subdirectory(http)

# 指定生成目标
add_executable(ssxrver main.cpp)

# 添加链接库
target_link_libraries(ssxrver NetFunctions)
target_link_libraries(ssxrver BaseFunctions)
target_link_libraries(ssxrver NetFunctions)
target_link_libraries(ssxrver HttpFunctions)

