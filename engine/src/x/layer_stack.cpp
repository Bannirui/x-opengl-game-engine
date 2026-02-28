//
// Created by rui ding on 2026/2/25.
//

#include "x/layer_stack.h"

#include "x/core/layer.h"

LayerStack::LayerStack() {}

LayerStack::~LayerStack()
{
    for (Layer *layer : m_layers)
    {
        layer->OnDetach();
        delete layer;
    }
}

void LayerStack::PushLayer(Layer *layer)
{
    m_layers.emplace(m_layers.begin() + m_layerInsertIndex, layer);
    ++m_layerInsertIndex;
    layer->OnAttach();
}

void LayerStack::PushOverlay(Layer *overlay)
{
    m_layers.emplace_back(overlay);
    overlay->OnAttach();
}

void LayerStack::PopLayer(Layer *layer)
{
    // 只在分隔区间内检索
    if (auto it = std::find(m_layers.begin(), m_layers.begin() + m_layerInsertIndex, layer);
        it != m_layers.begin() + m_layerInsertIndex)
    {
        layer->OnDetach();
        m_layers.erase(it);
        --m_layerInsertIndex;
    }
}

void LayerStack::PopOverlay(Layer *overlay)
{
    if (auto it = std::find(m_layers.begin(), m_layers.end(), overlay); it != m_layers.end())
    {
        overlay->OnDetach();
        m_layers.erase(it);
    }
}
