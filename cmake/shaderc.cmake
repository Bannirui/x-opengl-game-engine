include(FetchContent)

# SPIRV-Headers
FetchContent_Declare(
        spirv-headers
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
        GIT_TAG        main
)

# SPIRV-Tools
FetchContent_Declare(
        spirv-tools
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools.git
        GIT_TAG        main
)

# glslang
FetchContent_Declare(
        glslang
        GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
        GIT_TAG        main
)

# shaderc
FetchContent_Declare(
        shaderc
        GIT_REPOSITORY https://github.com/google/shaderc.git
        GIT_TAG        main
)

# 声明SPIRV-Cross
FetchContent_Declare(
        spirv-cross
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Cross.git
        GIT_TAG main
)

# 关闭测试并让shaderc知道依赖已经就绪
set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)

# 告诉shaderc不要去third_party找 而是用我们上面下载的
set(SHADERC_SPIRV_TOOLS_DIR "${spirv-tools_SOURCE_DIR}" CACHE PATH "" FORCE)
set(SHADERC_GLSLANG_DIR "${glslang_SOURCE_DIR}" CACHE PATH "" FORCE)

set(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS ON CACHE BOOL "" FORCE)
set(SPIRV_CROSS_SHARED OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_CLI OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(spirv-headers spirv-tools glslang shaderc spirv-cross)