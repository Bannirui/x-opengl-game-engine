include(FetchContent)

# ——— shaderc prerequisites (populate their SOURCE_DIR first) ———

FetchContent_Declare(
        spirv-headers
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
        GIT_TAG        8c5559c134abcf432ec59db842404087b9906c1a # main HEAD 2026-05-20
        GIT_PROGRESS TRUE
)

FetchContent_Declare(
        spirv-tools
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools.git
        GIT_TAG        bf3ad6e795df95455c206452ce78a6c0277a5dd3 # main HEAD 2026-05-21
        GIT_PROGRESS TRUE
)

FetchContent_Declare(
        glslang
        GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
        GIT_TAG        5313f0a5b1ef12450fc4b70435a1d03a4de23ddd # main HEAD 2026-05-22
        GIT_PROGRESS TRUE
)

# Make these available now so SOURCE_DIR variables exist
FetchContent_MakeAvailable(spirv-headers spirv-tools glslang)

# ——— shaderc (reuse the dependencies above) ———

set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
set(SHADERC_SPIRV_TOOLS_DIR "${spirv-tools_SOURCE_DIR}" CACHE PATH "" FORCE)
set(SHADERC_GLSLANG_DIR "${glslang_SOURCE_DIR}" CACHE PATH "" FORCE)

FetchContent_Declare(
        shaderc
        GIT_REPOSITORY https://github.com/google/shaderc.git
        GIT_TAG        b16fb67935326f7ea1ead8bd2b131608b4148230 # main HEAD 2026-05-14
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(shaderc)

# ——— spirv-cross (used by the engine for shader reflection) ———

FetchContent_Declare(
        spirv-cross
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Cross.git
        GIT_TAG 38681a30e09679191cc3957719eeee76024f6daf # main HEAD 2026-05-20
        GIT_PROGRESS TRUE
)

set(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS ON CACHE BOOL "" FORCE)
set(SPIRV_CROSS_SHARED OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_CLI OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(spirv-cross)
