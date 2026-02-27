//
// Created by rui ding on 2026/2/25.
//

#pragma once

#include "pch.h"

class Layer;

// 用栈的方式管理不同的系统模块 用指针分层管理 普通层和覆盖层
// 普通层最先渲染
// 覆盖层最先接收事件
class LayerStack
{
public:
    LayerStack();
    ~LayerStack();

    // 向系统添加普通层模块 放普通模块化的时候往分层点放
    void PushLayer(Layer *layer);
    // 向系统添加覆盖层模块 往最后放
    void PushOverlay(Layer *overlay);
    // 从系统移除普通层模块
    void PopLayer(Layer *layer);
    // 从系统移除覆盖层模块
    void PopOverlay(Layer *overlay);

    std::vector<Layer *>::iterator begin() { return m_layers.begin(); }
    std::vector<Layer *>::iterator end() { return m_layers.end(); }

private:
    std::vector<Layer *> m_layers;
    // 隔开普通层和覆盖层 前面是普通层 后面是覆盖层
    uint32_t m_layerInsertIndex{0};
};
