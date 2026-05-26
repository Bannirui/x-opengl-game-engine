//
// Created by dingrui on 2/23/26.
//

#pragma once

#include "pch.h"
#include "x/core/base.h"

// 事件枚举
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

// 事件类型枚举 层看自己感不感兴趣就看标识里面有没有这个类型
enum EventCategory
{
    kNone                     = 0,
    kEventCategoryApplication = BIT(0),
    kEventCategoryInput       = BIT(1),
    kEventCategoryKeyboard    = BIT(2),
    kEventCategoryMouse       = BIT(3),
    kEventCategoryMouseButton = BIT(4),
};

#define EVENT_CLASS_TYPE(type)                                                                      \
    static EventType GetStaticType() {                                                              \
        return EventType::type;                                                                     \
    }                                                                                               \
    virtual EventType GetEventType() const override {                                               \
        return GetStaticType();                                                                     \
    }                                                                                               \
    virtual const char* GetName() const override {                                                  \
        return #type;                                                                               \
    }                                                                                               \
    std::unique_ptr<Event> Clone() const override {                                                 \
        return std::make_unique<std::remove_cv_t<std::remove_reference_t<decltype(*this)>>>(*this); \
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
    virtual ~Event() = default;

    virtual EventType   GetEventType() const = 0;
    virtual const char* GetName() const      = 0;
    virtual int         GetCategory() const  = 0;
    /**
     * 虚函数版本的拷贝构造 解决两个问题
     *   - 多态拷贝 拿到的是Event&基类引用 直接按值拷贝会切片丢失子类数据
     * Clone()通过虚函数分发到正确的子类make_unique<KeyPressEvent>(*this)
     *   - 生命周期 GLFW回调中事件在栈上临时构造 回调返回即销毁 Clone()将事件拷贝到堆上 所有权转移给事件队列
     * 在下一帧ProcessEvents中处理
     */
    virtual std::unique_ptr<Event> Clone() const = 0;

    virtual std::string ToString() const { return GetName(); }

    bool IsInCategory(EventCategory category) { return GetCategory() & category; }

protected:
    Event() = default;

public:
    bool Handled{false};
};

class EventDispatcher
{
public:
    EventDispatcher(Event& event) : m_event(event) {}

    template <typename T, typename F>
    bool Dispatch(const F& func)
    {
        if (m_event.GetEventType() == T::GetStaticType())
        {
            m_event.Handled |= func(static_cast<T&>(m_event));
            return true;
        }
        return false;
    }

private:
    Event& m_event;
};

inline std::ostream& operator<<(std::ostream& os, const Event& e)
{
    return os << e.ToString();
}

// 整合spdlog的格式化输出 支持Event的所有派生类
template <typename T>
struct fmt::formatter<T, std::enable_if_t<std::is_base_of<Event, T>::value, char>> : fmt::formatter<std::string>
{
    auto format(const Event& e, format_context& ctx) const
    {
        return fmt::formatter<std::string>::format(e.ToString(), ctx);
    }
};