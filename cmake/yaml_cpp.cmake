include(FetchContent)

FetchContent_Declare(
        yaml-cpp
        GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
        GIT_TAG 4861d049534ed6f2c51c45b01d7c2926022e5f3f # master HEAD 2026-04-20
        GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(yaml-cpp)