//
// Created by dingrui on 2/23/26.
//

#pragma once

#include "pch.h"
#include "x/core/base.h"

// 事件枚举
enum class EventType {
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

// 事件类别枚举 层看自己感不感兴趣就看标识里面有没有这个类型
enum EventCategory {
    kNone = 0,
    kEventCategoryApplication = BIT(0),
    kEventCategoryInput = BIT(1),
    kEventCategoryKeyboard = BIT(2),
    kEventCategoryMouse = BIT(3),
    kEventCategoryMouseButton = BIT(4),
};

// 每个事件的名称 用来给事件本身去获取 比如打印在日志里面
constexpr const char* EventTypeName(EventType type) {
    switch (type) {
        case EventType::kNone:
            return "None";
        case EventType::kWindowClose:
            return "WindowClose";
        case EventType::kWindowResize:
            return "WindowResize";
        case EventType::kWindowFocus:
            return "WindowFocus";
        case EventType::kWindowLostFocus:
            return "WindowLostFocus";
        case EventType::kWindowMoved:
            return "WindowMoved";
        case EventType::kAppTick:
            return "AppTick";
        case EventType::kAppUpdate:
            return "AppUpdate";
        case EventType::kAppRender:
            return "AppRender";
        case EventType::kKeyPressed:
            return "KeyPressed";
        case EventType::kKeyReleased:
            return "KeyReleased";
        case EventType::kKeyTyped:
            return "KeyTyped";
        case EventType::kMouseButtonPressed:
            return "MouseButtonPressed";
        case EventType::kMouseButtonReleased:
            return "MouseButtonReleased";
        case EventType::kMouseMoved:
            return "MouseMoved";
        case EventType::kMouseScrolled:
            return "MouseScrolled";
    }
    return "Unknown";
}

// 事件的基类
class Event {
    friend class EventDispatcher;

public:
    virtual ~Event() = default;

    virtual EventType GetEventType() const = 0;
    virtual const char* GetName() const = 0;
    virtual int GetCategory() const = 0;
    /**
     * 虚函数版本的拷贝构造 解决两个问题
     *   - 多态拷贝 拿到的是Event&基类引用 直接按值拷贝会切片丢失子类数据
     * Clone()通过虚函数分发到正确的子类make_unique<KeyPressEvent>(*this)
     *   - 生命周期 GLFW回调中事件在栈上临时构造 回调返回即销毁 Clone()将事件拷贝到堆上 所有权转移给事件队列
     * 在下一帧ProcessEvents中处理
     */
    virtual std::unique_ptr<Event> Clone() const = 0;

    virtual std::string ToString() const {
        return GetName();
    }

    // 判断事件属于的类型
    bool IsInCategory(EventCategory category) const {
        return GetCategory() & category;
    }

protected:
    Event() = default;

public:
    // 事件上的标识 标识事件有没有被处理过 引擎层是分层的 ImGui层在LayerStack的分割线后面会优先处理鼠标/键盘事件
    // 键鼠事件一旦被处理过就打上被处理过的标识 保证ImGui层比渲染层可以优先吞掉键鼠事件
    bool Handled{false};
};

/**
 * CRTP技术
 * 每个事件子类继承此模板 编译期自动生成GetEventType/GetName/GetCategory/Clone这几个函数
 *
 * @tparam Derived   具体事件类型 比如KeyPressEvent
 * @tparam eType     事件类型枚举值
 * @tparam eCategory 事件类别位掩码
 * @tparam Base      中间基类 如KeyEvent提供m_keyCode 默认直接继承Event
 */
template <typename Derived, EventType eType, int eCategory, typename Base = Event>
class EventImpl : public Base {
    static_assert(std::is_base_of_v<Event, Base>, "Base must derive from Event");

public:
    using Base::Base;

    // 事件类型
    static constexpr EventType StaticType = eType;
    // 事件分类
    static constexpr int Category = eCategory;

    EventType GetEventType() const final {
        return eType;
    }

    const char* GetName() const final {
        return EventTypeName(eType);
    }

    int GetCategory() const final {
        return eCategory;
    }

    std::unique_ptr<Event> Clone() const final {
        return std::make_unique<Derived>(static_cast<const Derived&>(*this));
    }
};

class EventDispatcher {
public:
    EventDispatcher(Event& event) : m_event(event) {}

    template <typename T, typename F>
    bool Dispatch(const F& func) {
        if (m_event.GetEventType() == T::StaticType) {
            m_event.Handled |= func(static_cast<T&>(m_event));
            return true;
        }
        return false;
    }

private:
    Event& m_event;
};

inline std::ostream& operator<<(std::ostream& os, const Event& e) {
    return os << e.ToString();
}

// 整合spdlog的格式化输出 支持Event的所有派生类
template <typename T>
struct fmt::formatter<T, std::enable_if_t<std::is_base_of<Event, T>::value, char>> : fmt::formatter<std::string> {
    auto format(const Event& e, format_context& ctx) const {
        return fmt::formatter<std::string>::format(e.ToString(), ctx);
    }
};
