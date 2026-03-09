include(FetchContent)

set(BOX2D_BUILD_TESTBED OFF)
set(BOX2D_BUILD_UNIT_TESTS OFF)
set(BOX2D_SAMPLES OFF)

FetchContent_Declare(
        box2d
        GIT_REPOSITORY https://github.com/erincatto/box2d.git
        GIT_TAG        v2.4.1
        GIT_PROGRESS   TRUE
)

FetchContent_MakeAvailable(box2d)