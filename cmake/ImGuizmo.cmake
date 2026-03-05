include(FetchContent)

FetchContent_Declare(
        ImGuizmo
        GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo.git
        GIT_TAG master
        GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(ImGuizmo)

add_library(ImGuizmo
        ${imguizmo_SOURCE_DIR}/ImGuizmo.cpp
)

# zmo依赖imgui
target_link_libraries(ImGuizmo PUBLIC imgui)

target_include_directories(ImGuizmo PUBLIC
        ${imguizmo_SOURCE_DIR}
)