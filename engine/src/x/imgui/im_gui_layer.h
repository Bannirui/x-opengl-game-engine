//
// Created by rui ding on 2026/2/26.
//

#pragma once

#include "x/layer.h"

class ImGuiLayer : public Layer
{
public:
    ImGuiLayer();
    ~ImGuiLayer() override;

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate() override;
    void OnEvent(const Event &e) override;

private:
    float m_time{0.0f};
};
