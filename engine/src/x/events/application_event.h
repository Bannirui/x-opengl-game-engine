//
// Created by dingrui on 2/23/26.
//

#pragma once

#include "x/events/event.h"

class WindowResizeEvent : public Event
{
public:
    WindowResizeEvent(uint32_t width, uint32_t height) : m_width(width), m_height(height) {}

    uint32_t get_width() const { return m_width; }
    uint32_t get_height() const { return m_height; }

    std::string ToString() const override
    {
        std::stringstream ss;
        ss << "WindowResizeEvent: " << m_width << ", " << m_height;
        return ss.str();
    }

    EVENT_CLASS_TYPE(kWindowResize)
    EVENT_CLASS_CATEGORY(kEventCategoryApplication)

private:
    uint32_t m_width, m_height;
};

class WindowCloseEvent : public Event
{
public:
    WindowCloseEvent() = default;

    EVENT_CLASS_TYPE(kWindowClose)
    EVENT_CLASS_CATEGORY(kEventCategoryApplication)
};

class AppTickEvent : public Event
{
public:
    AppTickEvent() = default;

    EVENT_CLASS_TYPE(kAppTick)
    EVENT_CLASS_CATEGORY(kEventCategoryApplication)
};

class AppUpdateEvent : public Event
{
public:
    AppUpdateEvent() = default;

    EVENT_CLASS_TYPE(kAppUpdate)
    EVENT_CLASS_CATEGORY(kEventCategoryApplication)
};

class AppRenderEvent : public Event
{
public:
    AppRenderEvent() = default;

    EVENT_CLASS_TYPE(kAppRender)
    EVENT_CLASS_CATEGORY(kEventCategoryApplication)
};
