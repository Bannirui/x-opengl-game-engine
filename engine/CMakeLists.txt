# 源文件
file(GLOB_RECURSE SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
set(HDR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
file(GLOB_RECURSE HDRS
        "${HDR_DIR}/*.h"
        "${HDR_DIR}/*.hpp"
)

# 添加头文件的目的仅仅是为了让IDEA识别到 新建文件的时候添加索引
add_library(${LIB_NAME} STATIC
        ${HDRS}
        ${SRCS}
)

target_include_directories(${LIB_NAME}
    PUBLIC
        ${HDR_DIR}
)

target_link_libraries(${LIB_NAME}
    PUBLIC spdlog
    PUBLIC glad
    PUBLIC glfw
)

target_precompile_headers(${LIB_NAME}
    PRIVATE ${HDR_DIR}/pch.h
)

# 要用OpenGL的版本 从cmake的pre set传进来
target_compile_definitions(${LIB_NAME} PUBLIC
        OPENGL_VERSION_MAJOR=${OPENGL_VERSION_MAJOR}
        OPENGL_VERSION_MINOR=${OPENGL_VERSION_MINOR}
)
