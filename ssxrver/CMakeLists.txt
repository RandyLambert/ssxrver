#CMake 最低版本好要求
cmake_minimum_required (VERSION 3.11.2)

#项目信息
project(web_ssxrver)

# 查找当前目录下的所有源文件
# 并将名称保存到DIR——SRCS变量
aux_source_directory(. DIR_SRCS)

# 添加net http base子目录
add_sudbirectory(net)
add_sudbirectory(http)
add_sudbirectory(base)

# 指定生成目标
add_executable(ssxrver main.cpp)

# 添加链接库
target_link_libraries(ssxrver BaseFunctions)
target_link_libraries(ssxrver NetFunctions)
target_link_libraries(ssxrver HttpFunctions)

