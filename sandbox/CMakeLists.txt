# 源文件
file(GLOB_RECURSE SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.*")

set(APP_NAME "sandbox-app")
add_executable(${APP_NAME} ${SRCS})

target_link_libraries(${APP_NAME}
    PRIVATE ${LIB_NAME}
)