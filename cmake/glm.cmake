# 依赖FetchContent管理三方库
include(FetchContent)
# glm
FetchContent_Declare(
        glm
        GIT_REPOSITORY	https://github.com/icaven/glm.git
        GIT_TAG 	    dca38025fba63bb9284023e6de55f756b9e37cec
        GIT_PROGRESS TRUE
)
# 下载依赖的源码
FetchContent_MakeAvailable(glm)
# 启用glm中实验性质的模块
add_definitions(-DGLM_ENABLE_EXPERIMENTAL)
