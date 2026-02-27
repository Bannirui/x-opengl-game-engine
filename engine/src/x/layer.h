//
// Created by rui ding on 2026/2/25.
//

#pragma once
#include "events/key_event.h"

class Event;

// 把不同的系统组合变成可插拔的模块化 不用过度耦合在一起 再用栈管理成责任链模式
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
    virtual void OnUpdate() {}
    virtual void OnImguiRender() {}
    /**
     * 系统向各模块发布事件
     * @param e 系统发布的是什么事件
     */
    virtual void OnEvent(Event &e) {}

    inline const std::string &get_name() const { return m_debugName; }

protected:
    std::string m_debugName;
};
