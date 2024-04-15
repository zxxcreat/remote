
file(GLOB SOURCE_FILES "*.c" "*.cpp" "*.cc")
# 获取当前目录的完整路径
get_filename_component(DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)



# 创建可执行文件或库

add_library(${DIR_NAME} SHARED ${SOURCE_FILES})

file(GLOB_RECURSE ALL_SO_FILES "${PROJECT_SOURCE_DIR}/so/*.so")
file(GLOB_RECURSE ALL_A_FILES "${PROJECT_SOURCE_DIR}/a/*.a")


# 链接静态库和动态库
# 链接找到的所有 .so 文件
foreach(SO_FILE IN LISTS ALL_SO_FILES)
    target_link_libraries(${DIR_NAME} ${SO_FILE})
endforeach()

# 链接找到的所有 .a 文件
foreach(A_FILE IN LISTS ALL_A_FILES)
    target_link_libraries(${DIR_NAME} ${A_FILE})
endforeach()

# 设置动态库的RPATH
set_target_properties(${DIR_NAME} PROPERTIES
     BUILD_RPATH "$ORIGIN/../so"
    INSTALL_RPATH "$ORIGIN/../so"
)

