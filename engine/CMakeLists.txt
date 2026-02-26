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
    PUBLIC imgui
)

target_precompile_headers(${LIB_NAME}
    PRIVATE ${HDR_DIR}/pch.h
)

if(APPLE)
    target_compile_definitions(${LIB_NAME}
        # OpenGL版本
        PRIVATE OPENGL_VERSION_MAJOR=3
                OPENGL_VERSION_MINOR=3
    )
endif()

if(UNIX AND NOT APPLE)
    target_compile_definitions(${LIB_NAME}
        PRIVATE OPENGL_VERSION_MAJOR=4
                OPENGL_VERSION_MINOR=5
    )
endif()

# 系统构建类型在pre set中指定 debug就启用X_DEBUG 在cpp中可以使用调试日志 release就不启用
target_compile_definitions(${LIB_NAME}
    PUBLIC
        $<$<CONFIG:Debug>:X_DEBUG>
        $<$<CONFIG:Release>:X_RELEASE>
)