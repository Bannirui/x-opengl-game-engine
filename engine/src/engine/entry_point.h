//
// Created by dingrui on 2/23/26.
//
#pragma once

extern XApplication* CreateApplication();

int main(int argc, char** argv) {
    auto app = CreateApplication();
    app->Run();
    delete app;
    return 0;
}
