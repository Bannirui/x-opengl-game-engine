include(FetchContent)

# Disable unnecessary parts
set(SHADERC_SKIP_TESTS ON)
set(SHADERC_SKIP_EXAMPLES ON)
set(SHADERC_SKIP_INSTALL ON)

# SPIRV-Headers
FetchContent_Declare(
        spirv-headers
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
        GIT_TAG vulkan-sdk-1.3.296.0
        GIT_PROGRESS TRUE
)

# Fetch SPIRV-Tools first
FetchContent_Declare(
        spirv-tools
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools.git
        GIT_TAG vulkan-sdk-1.3.296.0
        GIT_PROGRESS TRUE
)

FetchContent_Declare(
        spirv-cross
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Cross.git
        GIT_TAG vulkan-sdk-1.3.296.0
        GIT_PROGRESS TRUE
)

# glslang
FetchContent_Declare(
        glslang
        GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
        GIT_TAG vulkan-sdk-1.3.296.0
        GIT_PROGRESS TRUE
)

# Fetch Shaderc
FetchContent_Declare(
        shaderc
        GIT_REPOSITORY https://github.com/google/shaderc.git
        GIT_TAG v2023.7
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
        GIT_SUBMODULES_RECURSE TRUE
)

FetchContent_MakeAvailable(
        spirv-headers
        spirv-tools
        spirv-cross
        glslang
        shaderc
)
