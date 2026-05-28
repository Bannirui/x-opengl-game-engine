//
// Created by rui ding on 2026/2/25.
//

#pragma once

#include "x/core/base.h"

#include <cstdint>
#include <vector>

class Layer;

/**
 * 引擎采用分层思想 每层{@see Layer}实现OnUpdate/OnEvent/OnImguiRender
 * ImGui先消费事件
 *   - 鼠标点击
 *   - 键盘
 * 如果ImGui处理了事件 事件的Handled就被标记true 不再传给游戏层 这就是为什么ImGui窗口能拦截鼠标/键盘输入
 *
 * LayerStack本质是一个有分割线的Vector
 */
class LayerStack {
public:
    LayerStack();
    ~LayerStack();

    /**
     * 插入位置是分割线
     * 添加结束之后的效果是 被放在了分割线左边
     */
    void PushLayer(X::Scope<Layer> layer);
    /**
     * 插入位置是Vector的最后
     * 添加结束之后的效果是 被放在了分割线右边
     */
    void PushOverlay(X::Scope<Layer> overlay);
    void PopLayer(Layer* layer);
    void PopOverlay(Layer* overlay);

    std::vector<X::Scope<Layer>>::iterator begin() {
        return m_layers.begin();
    }

    std::vector<X::Scope<Layer>>::iterator end() {
        return m_layers.end();
    }

    std::vector<X::Scope<Layer>>::reverse_iterator rbegin() {
        return m_layers.rbegin();
    }

    std::vector<X::Scope<Layer>>::reverse_iterator rend() {
        return m_layers.rend();
    }

    std::vector<X::Scope<Layer>>::const_iterator cbegin() const {
        return m_layers.begin();
    }

    std::vector<X::Scope<Layer>>::const_iterator cend() const {
        return m_layers.end();
    }

    std::vector<X::Scope<Layer>>::const_reverse_iterator rbegin() const {
        return m_layers.rbegin();
    }

    std::vector<X::Scope<Layer>>::const_reverse_iterator rend() const {
        return m_layers.rend();
    }

private:
    // layers的分割线 左边是逻辑层 右边是ImGui层
    uint32_t m_layerInsertIndex{0};
    // 存放所有的层
    std::vector<X::Scope<Layer>> m_layers;
};
