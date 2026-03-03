//
// Created by dingrui on 2/23/26.
//
#pragma once

extern XApplication *CreateApplication();

int main(int argc, char **argv)
{
    XLog::Init();

    X_PROFILE_BEGIN_SESSION("Startup", "asset/output/XProfile-Startup.json");
    auto app = CreateApplication();
    X_PROFILE_END_SESSION();

    X_PROFILE_BEGIN_SESSION("Runtime", "asset/output/XProfile-Runtime.json");
    app->run();
    X_PROFILE_END_SESSION();

    X_PROFILE_BEGIN_SESSION("Shutdown", "asset/output/XProfile-Shutdown.json");
    delete app;
    X_PROFILE_END_SESSION();
    return 0;
}
