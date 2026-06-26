//
// Created by dingrui on 2/23/26.
//

#pragma once

#include "x/core/mouse_codes.h"
#include "x/events/event.h"

// 鼠标移动
class MouseMovedEvent
    : public EventImpl<MouseMovedEvent, EventType::kMouseMoved, kEventCategoryMouse | kEventCategoryInput> {
public:
    MouseMovedEvent(const float x, const float y) : m_mouseX(x), m_mouseY(y) {}

    float get_x() const {
        return m_mouseX;
    }

    float get_y() const {
        return m_mouseY;
    }

    std::string ToString() const override {
        std::stringstream ss;
        ss << GetName() << ": 鼠标移动到(" << m_mouseX << ", " << m_mouseY << ")";
        return ss.str();
    }

private:
    float m_mouseX, m_mouseY;
};

// 鼠标滚轮
class MouseScrolledEvent
    : public EventImpl<MouseScrolledEvent, EventType::kMouseScrolled, kEventCategoryMouse | kEventCategoryInput> {
public:
    MouseScrolledEvent(const float xOffset, const float yOffset) : m_xOffset(xOffset), m_yOffset(yOffset) {}

    float get_xOffset() const {
        return m_xOffset;
    }

    float get_yOffset() const {
        return m_yOffset;
    }

    std::string ToString() const override {
        std::stringstream ss;
        std::string horizontal = m_xOffset > 0.0f ? "向右" : "向左";
        std::string vertical = m_yOffset > 0.0f ? "向前" : "向后";
        ss << GetName() << ": 滚轮 " << horizontal << ", " << vertical;
        return ss.str();
    }

private:
    /**
     * 鼠标中键
     *   - 前后
     *     - 向前滚 >0
     *     - 向后滚 <0
     *   - 左右
     *     - 向左 <0
     *     - 向右 >0
     */
    float m_xOffset, m_yOffset;
};

// 鼠标点击
class MouseButtonEvent : public Event {
public:
    MouseCode get_mouseButton() const {
        return m_button;
    }

protected:
    MouseButtonEvent(const MouseCode button) : m_button(button) {}

protected:
    MouseCode m_button;
};

// 鼠标点击
class MouseButtonPressedEvent
    : public EventImpl<MouseButtonPressedEvent, EventType::kMouseButtonPressed,
                       kEventCategoryMouse | kEventCategoryInput | kEventCategoryMouseButton, MouseButtonEvent> {
public:
    MouseButtonPressedEvent(const MouseCode button) : EventImpl(button) {}

    std::string ToString() const override {
        std::stringstream ss;
        ss << GetName() << ": 点击" << m_button;
        return ss.str();
    }
};

// 鼠标释放
class MouseButtonReleasedEvent
    : public EventImpl<MouseButtonReleasedEvent, EventType::kMouseButtonReleased,
                       kEventCategoryMouse | kEventCategoryInput | kEventCategoryMouseButton, MouseButtonEvent> {
public:
    MouseButtonReleasedEvent(const MouseCode button) : EventImpl(button) {}

    std::string ToString() const override {
        std::stringstream ss;
        ss << GetName() << ": 释放" << m_button;
        return ss.str();
    }
};
