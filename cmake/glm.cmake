# 依赖FetchContent管理三方库
include(FetchContent)
# glm
FetchContent_Declare(
        glm
        GIT_REPOSITORY	https://github.com/g-truc/glm.git
        GIT_TAG 	    1.0.1
        GIT_PROGRESS TRUE
)
# 下载依赖的源码
FetchContent_MakeAvailable(glm)
# 启用glm中实验性质的模块
target_compile_definitions(glm INTERFACE GLM_ENABLE_EXPERIMENTAL)
