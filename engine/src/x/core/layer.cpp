//
// Created by rui ding on 2026/2/25.
//

#include "x/core/layer.h"
#include "x/events/event.h"

Layer::Layer(const std::string &name) : m_debugName(name) {}

bool Layer::IsInterestedIn(const Event& e) const
{
    return m_eventMask & e.GetCategory();
}
