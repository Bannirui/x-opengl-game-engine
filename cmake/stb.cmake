# 依赖FetchContent管理三方库
include(FetchContent)
# stb
FetchContent_Declare(
        stb
        GIT_REPOSITORY	https://github.com/nothings/stb.git
        GIT_TAG 	    f1c79c02822848a9bed4315b12c8c8f3761e1296
        GIT_PROGRESS TRUE
)
# 下载依赖的源码
FetchContent_MakeAvailable(stb)
# stb本身是纯头文件 创建一个cpp文件引用stb头文件编译成静态库库给自己链接使用
set(STB_IMAGE_CPP "${CMAKE_CURRENT_BINARY_DIR}/stb_image_impl.cpp")
file(WRITE ${STB_IMAGE_CPP}
        "#define STB_IMAGE_IMPLEMENTATION\n#include \"stb_image.h\"\n")
add_library(stb STATIC ${STB_IMAGE_CPP})
target_include_directories(stb PUBLIC ${stb_SOURCE_DIR})
