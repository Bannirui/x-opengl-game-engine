# ECS实体组件系统
include(FetchContent)
FetchContent_Declare(
        entt
        GIT_REPOSITORY	https://github.com/skypjack/entt.git
        GIT_TAG 	    v3.13.x
        GIT_PROGRESS    TRUE
)
FetchContent_MakeAvailable(entt)
