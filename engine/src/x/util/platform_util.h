//
// Created by rui ding on 2026/3/5.
//

#pragma once

#include "pch.h"

class FileDialog
{
public:
    static std::string OpenFile(const char* filter);
    static std::string SaveFile(const char* filter);
};
