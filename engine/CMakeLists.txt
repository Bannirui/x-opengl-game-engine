add_asset_copy_target(${LIB_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/asset")

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

# 编译告警
target_compile_options(${LIB_NAME}
    PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wno-error=unused-parameter
    -Wno-deprecated-copy
)

# macOS Objective-C++
if(APPLE)
    target_sources(${LIB_NAME} PRIVATE
            src/platform/mac/mac_platform_util.mm
    )
    # cpp+pch+object-c不能搭配 不让mm文件使用pch
    set_source_files_properties(
            src/platform/mac/mac_platform_util.mm
            PROPERTIES
            SKIP_PRECOMPILE_HEADERS ON
            COMPILE_FLAGS "-Wno-deprecated-declarations"
    )
endif()

target_include_directories(${LIB_NAME}
    PUBLIC
        ${HDR_DIR}
)

target_link_libraries(${LIB_NAME}
    PUBLIC spdlog
        glad
        glfw
        imgui
        glm
        stb
        EnTT
        yaml-cpp
        imguizmo
        shaderc spirv-cross-core
        box2d
)

target_precompile_headers(${LIB_NAME}
    PRIVATE ${HDR_DIR}/pch.h
)

if(APPLE)
    target_compile_definitions(${LIB_NAME}
        # OpenGL版本
        PRIVATE X_GL_VERSION_MAJOR=3
                X_GL_VERSION_MINOR=3
    )
endif()

if(UNIX AND NOT APPLE)
    target_compile_definitions(${LIB_NAME}
        PRIVATE X_GL_VERSION_MAJOR=4
                X_GL_VERSION_MINOR=5
    )
endif()

# 系统构建类型在pre set中指定 debug就启用X_DEBUG 在cpp中可以使用调试日志 release就不启用
target_compile_definitions(${LIB_NAME}
    PRIVATE
        $<$<CONFIG:Debug>:X_ENABLE_ASSERTS> # Enable the debug option
)

add_dependencies(${LIB_NAME} ${ASSET_COPY_TARGET})
