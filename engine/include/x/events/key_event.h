//
// Created by dingrui on 2/23/26.
//

#pragma once
#include "x/core/key_codes.h"
#include "x/events/event.h"

class KeyEvent : public Event {
public:
    KeyCode get_keyCode() const {
        return m_keyCode;
    }

protected:
    KeyEvent(const int keycode) : m_keyCode(keycode) {}

protected:
    KeyCode m_keyCode;
};

class KeyPressEvent
    : public EventImpl<KeyPressEvent, EventType::kKeyPressed, kEventCategoryKeyboard | kEventCategoryInput, KeyEvent> {
public:
    KeyPressEvent(const KeyCode keycode, const bool isRepeat = false) : EventImpl(keycode), m_isRepeat(isRepeat) {}

    bool is_repeat() const {
        return m_isRepeat;
    }

    std::string ToString() const override {
        std::stringstream ss;
        ss << GetName() << ": " << m_keyCode << " (repeat = " << m_isRepeat << ")";
        return ss.str();
    }

private:
    bool m_isRepeat;
};

class KeyReleaseEvent : public EventImpl<KeyReleaseEvent, EventType::kKeyReleased,
                                         kEventCategoryKeyboard | kEventCategoryInput, KeyEvent> {
public:
    KeyReleaseEvent(const KeyCode keycode) : EventImpl(keycode) {}

    std::string ToString() const override {
        std::stringstream ss;
        ss << GetName() << ": " << m_keyCode;
        return ss.str();
    }
};

class KeyTypedEvent
    : public EventImpl<KeyTypedEvent, EventType::kKeyTyped, kEventCategoryKeyboard | kEventCategoryInput, KeyEvent> {
public:
    KeyTypedEvent(const KeyCode keycode) : EventImpl(keycode) {}

    std::string ToString() const override {
        std::stringstream ss;
        ss << GetName() << ": " << m_keyCode;
        return ss.str();
    }
};
