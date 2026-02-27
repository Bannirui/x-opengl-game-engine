//
// Created by rui ding on 2026/2/25.
//

#include "layer_stack.h"

#include "layer.h"

LayerStack::LayerStack() {}

LayerStack::~LayerStack()
{
    for (Layer *layer : m_layers)
    {
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
    if (auto it = std::find(m_layers.begin(), m_layers.end(), layer); it != m_layers.end())
    {
        m_layers.erase(it);
        --m_layerInsertIndex;
        layer->OnDetach();
    }
}

void LayerStack::PopOverlay(Layer *overlay)
{
    if (auto it = std::find(m_layers.begin(), m_layers.end(), overlay); it != m_layers.end())
    {
        m_layers.erase(it);
        overlay->OnDetach();
    }
}
