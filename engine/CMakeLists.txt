set(COPY_NAME "${LIB_NAME}-copy-assets")
# 拷贝资源文件到构建目录和运行目录
add_custom_target(${COPY_NAME} ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/asset" "${CMAKE_CURRENT_BINARY_DIR}/asset"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/asset" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/asset"
        COMMENT "Copying assets to build and runtime directories"
)
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
        ImGuizmo
        Vulkan::Vulkan
        shaderc glslang MachineIndependent OSDependent GenericCodeGen HLSL OGLCompiler SPIRV SPIRV-Tools SPIRV-Tools-opt spirv-cross-core spirv-cross-glsl
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
                X_GL_VERSION_MINOR=6
    )
endif()

# 系统构建类型在pre set中指定 debug就启用X_DEBUG 在cpp中可以使用调试日志 release就不启用
target_compile_definitions(${LIB_NAME}
    PRIVATE
        $<$<CONFIG:Debug>:X_ENABLE_ASSERTS> # Enable the debug option
)

# 构建时触发资源文件复制
add_dependencies(${LIB_NAME} ${COPY_NAME})
