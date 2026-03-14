include(FetchContent)

# 禁用 Box2D 自带的测试、单元测试和示例，防止重复定义 glad/glfw/imgui
set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BOX2D_SAMPLES OFF)

FetchContent_Declare(
        box2d
        GIT_REPOSITORY https://github.com/erincatto/box2d.git
        GIT_TAG        v2.4.1
        GIT_PROGRESS   TRUE
)

FetchContent_MakeAvailable(box2d)