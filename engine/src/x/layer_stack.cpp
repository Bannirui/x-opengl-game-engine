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
    // 插入到分割线位置 后面的元素整体右移
    m_layers.emplace(m_layers.begin() + m_layerInsertIndex, layer);
    // 后移分割线 保证左边跟右边被分割的语义
    ++m_layerInsertIndex;
}

void LayerStack::PushOverlay(Layer *overlay)
{
    // 直接丢到Vector的末尾
    m_layers.emplace_back(overlay);
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
    if (auto it = std::find(m_layers.begin() + m_layerInsertIndex, m_layers.end(), overlay); it != m_layers.end())
    {
        overlay->OnDetach();
        m_layers.erase(it);
    }
}
