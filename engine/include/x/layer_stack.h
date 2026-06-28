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
     * 添加游戏层
     * 插入位置是分割线
     * 添加结束之后的效果是 被放在了分割线左边
     */
    void PushLayer(Scope<Layer> layer);

    /**
     * 添加imgui层
     * 插入位置是Vector的最后
     * 添加结束之后的效果是 被放在了分割线右边
     */
    void PushOverlay(Scope<Layer> overlay);

    /**
     * 从分割线左边找到它 移除
     * @param layer 要移除的游戏层
     */
    void PopLayer(Layer* layer);

    /**
     * 从分割线右边找到它 移除
     * @param overlay 要移除的imgui层
     */
    void PopOverlay(Layer* overlay);

    std::vector<Scope<Layer>>::iterator begin() {
        return m_layers.begin();
    }

    std::vector<Scope<Layer>>::iterator end() {
        return m_layers.end();
    }

    std::vector<Scope<Layer>>::reverse_iterator rbegin() {
        return m_layers.rbegin();
    }

    std::vector<Scope<Layer>>::reverse_iterator rend() {
        return m_layers.rend();
    }

    std::vector<Scope<Layer>>::const_iterator cbegin() const {
        return m_layers.begin();
    }

    std::vector<Scope<Layer>>::const_iterator cend() const {
        return m_layers.end();
    }

    std::vector<Scope<Layer>>::const_reverse_iterator rbegin() const {
        return m_layers.rbegin();
    }

    std::vector<Scope<Layer>>::const_reverse_iterator rend() const {
        return m_layers.rend();
    }

private:
    // layers的分割线 前n个是逻辑层 左边是逻辑层[0...n) 右边是ImGui层[n...)
    uint32_t m_layerCnt{0};
    // 存放所有的层
    std::vector<Scope<Layer>> m_layers;
};
