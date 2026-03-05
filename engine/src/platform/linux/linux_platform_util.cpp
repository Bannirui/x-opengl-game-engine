//
// Created by rui ding on 2026/3/5.
//

#ifdef __linux__

    #include "x/util/platform_util.h"
    #include "pch.h"

    #include <cstdio>
    #include <memory>
    #include <array>

static std::string ExecCommand(const char* cmd)
{
    std::array<char, 256> buffer;
    std::string           result;

    FILE* pipe = popen(cmd, "r");
    if (!pipe) { return ""; }
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
    {
        result += buffer.data();
    }
    pclose(pipe);
    // remove newline
    if (!result.empty() && result.back() == '\n') { result.pop_back(); }
    return result;
}

std::string FileDialogs::OpenFile(const char* filter)
{
    std::string command = "zenity --file-selection";
    std::string path = ExecCommand(command.c_str());
    if (path.empty()) { return {}; }
    return path;
}

std::string FileDialogs::SaveFile(const char* filter)
{
    std::string command = "zenity --file-selection --save --confirm-overwrite";
    std::string path = ExecCommand(command.c_str());
    if (path.empty()) { return {}; }
    return path;
}

#endif
