# ECS实体组件系统
include(FetchContent)
FetchContent_Declare(
        entt
        GIT_REPOSITORY	https://github.com/skypjack/entt.git
        GIT_TAG 	    78213075654a688e9da6bc49f7f873d25c26d12c # v3.13.x HEAD 2024-04-10
        GIT_PROGRESS    TRUE
)
FetchContent_MakeAvailable(entt)
