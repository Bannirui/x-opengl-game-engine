//
// Created by rui ding on 2026/3/5.
//

#ifdef __APPLE__

    #include "x/util/platform_util.h"

    #import <Cocoa/Cocoa.h>

std::string FileDialog::OpenFile(const char* filter)
{
    @autoreleasepool
    {
        NSOpenPanel* panel = [NSOpenPanel openPanel];

        [panel setCanChooseFiles:YES];
        [panel setCanChooseDirectories:NO];
        [panel setAllowsMultipleSelection:NO];

        // 如果你以后想解析 filter 可以在这里做
        if ([panel runModal] == NSModalResponseOK)
        {
            NSURL* url = [[panel URLs] objectAtIndex:0];
            return std::string([[url path] UTF8String]);
        }
    }

    return {};
}

std::string FileDialog::SaveFile(const char* filter)
{
    @autoreleasepool
    {
        NSSavePanel* panel = [NSSavePanel savePanel];

        if ([panel runModal] == NSModalResponseOK)
        {
            NSURL* url = [panel URL];
            return std::string([[url path] UTF8String]);
        }
    }

    return {};
}

#endif
