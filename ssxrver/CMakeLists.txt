#CMake 最低版本好要求
cmake_minimum_required (VERSION 3.11.2)

set(CMAKE_BUILD_TYPE "Release")


if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall -Wno-unused-variable -pthread")
else( CMAKE_BUILD_TYPE STREQUAL "Debug" )
        set(CMAKE_BUILD_TYPE "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2 -pthread -fopenmp")
endif( CMAKE_BUILD_TYPE STREQUAL "Debug" )

message("Build Type:"${CMAKE_BUILD_TYPE} ${CMAKE_CXX_FLAGS})


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

if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
    endif()
endif()


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

