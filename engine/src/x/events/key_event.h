//
// Created by dingrui on 2/23/26.
//

#pragma once
#include "x/events/event.h"
#include "x/core/key_codes.h"

#include <imgui.h>

class KeyEvent : public Event {
public:
    KeyCode get_keyCode() const { return m_keyCode; }

    EVENT_CLASS_CATEGORY(kEventCategoryKeyboard | kEventCategoryInput);

protected:
    KeyEvent(const int keycode) : m_keyCode(keycode) {
    }

protected:
    KeyCode m_keyCode;
};

class KeyPressEvent : public KeyEvent {
public:
    KeyPressEvent(const int keycode, bool isRepeat = false) : KeyEvent(keycode), m_isRepeat(isRepeat) {
    }

    bool is_repeat() const { return m_isRepeat; }

    std::string ToString() const override {
        std::stringstream ss;
        ss << "KeyPressEvent: " << m_keyCode << " (repeat = " << m_isRepeat << ")";
        return ss.str();
    }

    EVENT_CLASS_TYPE(kKeyPressed)

private:
    // 键盘被重复按键
    bool m_isRepeat;
};

class KeyReleaseEvent : public KeyEvent {
public:
    KeyReleaseEvent(const int keycode) : KeyEvent(keycode) {
    }

    std::string ToString() const override {
        std::stringstream ss;
        ss << "KeyReleaseEvent: " << m_keyCode;
        return ss.str();
    }

    EVENT_CLASS_TYPE(kKeyReleased)
};

class KeyTypedEvent : public KeyEvent {
public:
    KeyTypedEvent(const int keycode) : KeyEvent(keycode) {
    }

    std::string ToString() const override {
        std::stringstream ss;
        ss << "KeyTypedEvent: " << m_keyCode;
        return ss.str();
    }

    EVENT_CLASS_TYPE(kKeyTyped)
};
