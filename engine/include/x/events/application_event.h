//
// Created by dingrui on 2/23/26.
//

#pragma once

#include "x/events/event.h"

// 窗体大小调整
class WindowResizeEvent : public EventImpl<WindowResizeEvent, EventType::kWindowResize, kEventCategoryApplication> {
public:
    WindowResizeEvent(uint32_t width, uint32_t height) : m_width(width), m_height(height) {}

    uint32_t get_width() const {
        return m_width;
    }

    uint32_t get_height() const {
        return m_height;
    }

    std::string ToString() const override {
        std::stringstream ss;
        ss << GetName() << ": 窗体大小调整到宽" << m_width << ", 高" << m_height;
        return ss.str();
    }

private:
    // 窗体大小被调整到新的尺寸
    uint32_t m_width, m_height;
};

// 关闭窗口
class WindowCloseEvent : public EventImpl<WindowCloseEvent, EventType::kWindowClose, kEventCategoryApplication> {
public:
    WindowCloseEvent() = default;

    std::string ToString() const override {
        std::stringstream ss;
        ss << GetName() << ": 关闭窗口";
        return ss.str();
    }
};

class AppTickEvent : public EventImpl<AppTickEvent, EventType::kAppTick, kEventCategoryApplication> {
public:
    AppTickEvent() = default;
};

class AppUpdateEvent : public EventImpl<AppUpdateEvent, EventType::kAppUpdate, kEventCategoryApplication> {
public:
    AppUpdateEvent() = default;
};

class AppRenderEvent : public EventImpl<AppRenderEvent, EventType::kAppRender, kEventCategoryApplication> {
public:
    AppRenderEvent() = default;
};
