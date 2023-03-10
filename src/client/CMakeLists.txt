#指定生成可执行文件ChatClient，包含了该目录下所有文件
aux_source_directory(. SRC_LIST)
add_executable(ChatClient ${SRC_LIST})

#生成的可执行文件链接时依赖的库
target_link_libraries(ChatClient pthread)