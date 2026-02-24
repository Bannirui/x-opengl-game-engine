# 依赖FetchContent管理三方库
include(FetchContent)
# glfw
FetchContent_Declare(
        glfw
        GIT_REPOSITORY	https://github.com/glfw/glfw.git
        GIT_TAG 	    3.4
        GIT_PROGRESS TRUE
)
# 下载依赖的源码
FetchContent_MakeAvailable(glfw)
