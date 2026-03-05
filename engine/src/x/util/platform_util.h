//
// Created by rui ding on 2026/3/5.
//

#pragma once

// 跨平台mac上是object-c pch+cpp+object-c的时候object-c不支持pch 要在make编译排除 直接不引用pch就行
#include <string>

class FileDialog
{
public:
    static std::string OpenFile(const char* filter);
    static std::string SaveFile(const char* filter);
};
