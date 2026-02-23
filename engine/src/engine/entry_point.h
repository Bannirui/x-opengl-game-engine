//
// Created by dingrui on 2/23/26.
//
#pragma once

extern XApplication *CreateApplication();

int main(int argc, char **argv) {
    XLog::Init();
    X_CORE_INFO("x-OpenGL-game-engine boost");

    auto app = CreateApplication();
    app->Run();
    delete app;
    return 0;
}
