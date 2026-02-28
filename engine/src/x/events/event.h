//
// Created by dingrui on 2/23/26.
//

#pragma once

#include "pch.h"

#include "x/core.h"

enum class EventType
{
    kNone = 0,
    kWindowClose,
    kWindowResize,
    kWindowFocus,
    kWindowLostFocus,
    kWindowMoved,
    kAppTick,
    kAppUpdate,
    kAppRender,
    kKeyPressed,
    kKeyReleased,
    kKeyTyped,
    kMouseButtonPressed,
    kMouseButtonReleased,
    kMouseMoved,
    kMouseScrolled,
};

enum EventCategory
{
    kNone                     = 0,
    kEventCategoryApplication = BIT(0),
    kEventCategoryInput       = BIT(1),
    kEventCategoryKeyboard    = BIT(2),
    kEventCategoryMouse       = BIT(3),
    kEventCategoryMouseButton = BIT(4),
};

#define EVENT_CLASS_TYPE(type)                      \
    static EventType GetStaticType()                \
    {                                               \
        return EventType::type;                     \
    }                                               \
    virtual EventType GetEventType() const override \
    {                                               \
        return GetStaticType();                     \
    }                                               \
    virtual const char *GetName() const override    \
    {                                               \
        return #type;                               \
    }

#define EVENT_CLASS_CATEGORY(category)       \
    virtual int GetCategory() const override \
    {                                        \
        return category;                     \
    }

class Event
{
    friend class EventDispatcher;

public:
    virtual EventType GetEventType() const = 0;

    virtual const char *GetName() const = 0;

    virtual int GetCategory() const = 0;

    virtual std::string ToString() const { return GetName(); }

    inline bool IsInCategory(EventCategory category) { return GetCategory() & category; }

protected:
    Event() = default;

public:
    bool Handled{false};
};

class EventDispatcher
{
public:
    EventDispatcher(Event &event) : m_event(event) {}

    template <typename T, typename F>
    bool Dispatch(const F& func)
    {
        if (m_event.GetEventType() == T::GetStaticType())
        {
            m_event.Handled = func(static_cast<T&>(m_event));
            return true;
        }
        return false;
    }

private:
    Event &m_event;
};

inline std::ostream &operator<<(std::ostream &os, const Event &e)
{
    return os << e.ToString();
}

// 整合spdlog的格式化输出 支持Event的所有派生类
template <typename T>
struct fmt::formatter<T, std::enable_if_t<std::is_base_of<Event, T>::value, char>> : fmt::formatter<std::string>
{
    auto format(const Event &e, format_context &ctx) const
    {
        return fmt::formatter<std::string>::format(e.ToString(), ctx);
    }
};