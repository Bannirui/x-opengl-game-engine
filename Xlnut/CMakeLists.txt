# 拷贝资源文件到构建目录和运行目录
add_custom_target(XLnut_copy_assets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/asset" "${CMAKE_CURRENT_BINARY_DIR}/asset"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/asset" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/asset"
        COMMENT "Copying assets to build and runtime directories"
)

# 源文件
file(GLOB_RECURSE SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.*")

set(APP_NAME "XLnut-app")
add_executable(${APP_NAME} ${SRCS})

target_link_libraries(${APP_NAME}
    PRIVATE ${LIB_NAME}
)

# 构建时触发资源文件复制
add_dependencies(${APP_NAME} XLnut_copy_assets)
