//
// Created by dingrui on 2/23/26.
//

#pragma once

#include "x/events/event.h"

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
        ss << GetName() << ": " << m_width << ", " << m_height;
        return ss.str();
    }

private:
    uint32_t m_width, m_height;
};

class WindowCloseEvent : public EventImpl<WindowCloseEvent, EventType::kWindowClose, kEventCategoryApplication> {
public:
    WindowCloseEvent() = default;
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
