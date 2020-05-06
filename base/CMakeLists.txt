# 查找当前目录下的所有原文件
# 并将名称保存到DIR_LIB_SRCS变量中
aux_source_directory(. DIR_LIB_SRCS_BASE)

# 生成链接库
add_library (BaseFunctions ${DIR_LIB_SRCS_BASE})

