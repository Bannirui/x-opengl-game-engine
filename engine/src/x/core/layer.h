//
// Created by rui ding on 2026/2/25.
//

#pragma once

#include "pch.h"

#include "x/core/timestep.h"

class Event;

/**
 * {@see Layer}+{@see LayerStack}组合成引擎的分层系统
 * Layer是引擎的最小组合单元
 * 任何的功能模块都是Layer的派生
 *   - 渲染
 *   - 音频
 *   - 物理
 *   - UI
 */
class Layer
{
public:
    Layer(const std::string &name = "Layer");
    virtual ~Layer() = default;

    /**
     * 不同的模块组合进系统
     */
    virtual void OnAttach() {}
    /**
     * 从系统中移除模块
     */
    virtual void OnDetach() {}
    /**
     * 系统向各模块发布渲染
     */
    virtual void OnUpdate(Timestep ts) {}
    /**
     * 每帧都要绘制Imgui的调试面板 各层怎么绘制各个模块自己决定这个函数的实现
     * Imgui的渲染独立于OnUpdate的绘制 确保先更新Imgui再更新UI逻辑
     */
    virtual void OnImguiRender() {}
    /**
     * 系统向各模块发布事件
     * @param e 系统发布的是什么事件
     */
    virtual void OnEvent(Event &e) {}

    /**
     * 用事件掩码看看是不是自己感兴趣的事件
     * @param e 分发的事件 每个Layer看看是不是自己感兴趣的事件
     */
    bool IsInterestedIn(const Event& e) const;

    const std::string &get_name() const { return m_debugName; }

protected:
    std::string m_debugName;
    // 标识当前层对什么事件感兴趣 默认对所有事件都感兴趣
    int m_eventMask = ~0;
};
