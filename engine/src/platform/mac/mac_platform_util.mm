//
// Created by rui ding on 2026/3/5.
//

#ifdef __APPLE__

    #include "x/util/platform_util.h"

    #import <Cocoa/Cocoa.h>

std::optional<std::string> FileDialog::OpenFile(const char* filter)
{
    @autoreleasepool
    {
        NSOpenPanel* panel = [NSOpenPanel openPanel];
        [panel setCanChooseFiles:YES];
        [panel setCanChooseDirectories:NO];
        [panel setAllowsMultipleSelection:NO];
        NSInteger result = [panel runModal];
        if (result == NSModalResponseOK)
        {
            NSURL* url = [[panel URLs] objectAtIndex:0];
            return std::string([[url path] UTF8String]);
        }
    }
    return nullptr;
}

std::optional<std::string> FileDialog::SaveFile(const char* filter)
{
    @autoreleasepool
    {
        NSSavePanel* panel = [NSSavePanel savePanel];
        [panel setCanCreateDirectories:YES];
        [panel setShowsTagField:NO];
        // 解析扩展名
        if (filter)
        {
            const char* ext = strchr(filter, '*');
            if (ext)
            {
                std::string extension = std::string(ext + 2); // 跳过*.
                NSString* nsExt = [NSString stringWithUTF8String:extension.c_str()];
                [panel setAllowedFileTypes:@[nsExt]];
            }
        }
        NSInteger result = [panel runModal];
        if (result == NSModalResponseOK)
        {
            NSURL* url = [panel URL];
            return std::string([[url path] UTF8String]);
        }
    }
    return nullptr;
}

#endif
