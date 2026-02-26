# 依赖FetchContent管理三方库
include(FetchContent)
# imgui
FetchContent_Declare(
        imgui
        GIT_REPOSITORY	https://github.com/ocornut/imgui.git
        GIT_TAG 	    v1.92.5
        GIT_PROGRESS TRUE
)
# 下载依赖的源码
FetchContent_MakeAvailable(imgui)
# ImGui本身不是用cmake组织的 需要手动编译成库
file(GLOB IMGUI_SOURCES
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw)
