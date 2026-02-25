//
// Created by rui ding on 2026/2/25.
//

#pragma once

class Event;

class Layer
{
public:
    Layer(const std::string &name = "Layer");
    virtual ~Layer();

    virtual void OnAttach() {}
    virtual void OnDetach() {}
    virtual void OnUpdate() {}
    virtual void OnEvent(const Event &e) {}

    inline const std::string &get_name() const { return m_debugName; }

protected:
    std::string m_debugName;
};
