set(APP_NAME "sandbox-app")
add_asset_copy_target(${APP_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/asset")

# 源文件
file(GLOB_RECURSE SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp")

add_executable(${APP_NAME} ${SRCS})

target_link_libraries(${APP_NAME}
    PRIVATE ${LIB_NAME}
)

add_dependencies(${APP_NAME} ${ASSET_COPY_TARGET})
