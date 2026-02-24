//
// Created by dingrui on 2/23/26.
//

#pragma once
#include "engine/events/event.h"
#include "engine/core/mouse_codes.h"

class MouseMovedEvent : public Event {
public:
    MouseMovedEvent(const float x, const float y) : m_mouseX(x), m_mouseY(y) {
    }

    float get_x() const { return m_mouseX; }
    float get_y() const { return m_mouseY; }

    std::string ToString() const override {
        std::stringstream ss;
        ss << "MouseMovedEvent: " << m_mouseX << ", " << m_mouseY;
        return ss.str();
    }

    EVENT_CLASS_TYPE(kMouseMoved)
    EVENT_CLASS_CATEGORY(kEventCategoryMouse | kEventCategoryInput)

private:
    float m_mouseX, m_mouseY;
};

class MouseScrolledEvent : public Event {
public:
    MouseScrolledEvent(const float xOffset, const float yOffset) : m_xOffset(xOffset), m_yOffset(yOffset) {
    }

    float get_xOffset() const { return m_xOffset; }
    float get_yOffset() const { return m_yOffset; }

    std::string ToString() const override {
        std::stringstream ss;
        ss << "MouseScrolledEvent: " << m_xOffset << ", " << m_yOffset;
        return ss.str();
    }

    EVENT_CLASS_TYPE(kMouseScrolled)
    EVENT_CLASS_CATEGORY(kEventCategoryMouse | kEventCategoryInput)

private:
    float m_xOffset, m_yOffset;
};

class MouseButtonEvent : public Event {
public:
    MouseCode get_mouseButton() const { return m_button; }

    EVENT_CLASS_CATEGORY(kEventCategoryMouse | kEventCategoryInput | kEventCategoryMouseButton)

protected:
    MouseButtonEvent(const uint8_t button) : m_button(button) {
    }

protected:
    MouseCode m_button;
};

class MouseButtonPressedEvent : public MouseButtonEvent {
public:
    MouseButtonPressedEvent(const uint8_t button) : MouseButtonEvent(button) {
    }

    std::string ToString() const override {
        std::stringstream ss;
        ss << "MouseButtonPressedEvent: " << m_button;
        return ss.str();
    }

    EVENT_CLASS_TYPE(kMouseButtonPressed)
};

class MouseButtonReleasedEvent : public MouseButtonEvent {
public:
    MouseButtonReleasedEvent(const uint8_t button) : MouseButtonEvent(button) {
    }

    std::string ToString() const override {
        std::stringstream ss;
        ss << "MouseButtonReleasedEvent: " << m_button;
        return ss.str();
    }

    EVENT_CLASS_TYPE(kMouseButtonReleased)
};
