include(FetchContent)

# spdlog
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY	https://github.com/gabime/spdlog.git
        GIT_TAG 	    v2.x
        GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(spdlog)
