//
// Created by rui ding on 2026/3/5.
//

#ifdef __linux__

    #include "x/util/platform_util.h"

    #include <cstdio>
    #include <array>
    #include <memory>
    #include <optional>

static std::optional<std::string> ExecCommand(const char* cmd)
{
    std::array<char, 256>                    buffer{};
    std::string                              result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe)
    {
        return std::nullopt;
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    if (result.empty())
    {
        return std::nullopt;
    }
    // remove trailing newline
    if (result.back() == '\n')
    {
        result.pop_back();
    }
    return result;
}

std::optional<std::string> FileDialog::OpenFile(const char* filter)
{
    (void)filter;  // avoid unused warning
    std::string command = "zenity --file-selection";
    return ExecCommand(command.c_str());
}

std::optional<std::string> FileDialog::SaveFile(const char* filter)
{
    (void)filter;  // avoid unused warning
    std::string command = "zenity --file-selection --save --confirm-overwrite";
    return ExecCommand(command.c_str());
}

#endif
