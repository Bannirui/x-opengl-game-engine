include(FetchContent)

FetchContent_Declare(
        ImGuizmo
        GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo.git
        GIT_TAG be8aa4aeab86b402701c8c1df011bd8cd776760b # master HEAD 2026-05-16
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