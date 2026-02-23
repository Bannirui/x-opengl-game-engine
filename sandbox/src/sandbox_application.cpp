//
// Created by dingrui on 2/23/26.
//

#include <x_engine.h>

class Sandbox : public XApplication {
public:
    Sandbox() {
    }

    ~Sandbox() override {
    }
};

XApplication *CreateApplication() {
    return new Sandbox();
}
