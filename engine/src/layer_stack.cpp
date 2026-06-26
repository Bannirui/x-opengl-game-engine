//
// Created by rui ding on 2026/2/25.
//

#include "x/layer_stack.h"

#include "x/core/layer.h"

LayerStack::LayerStack() {}

LayerStack::~LayerStack() {
    for (auto& layer : m_layers) {
        layer->OnDetach();
    }
}

void LayerStack::PushLayer(X::Scope<Layer> layer) {
    m_layers.emplace(m_layers.begin() + m_layerCnt, std::move(layer));
    ++m_layerCnt;
}

void LayerStack::PushOverlay(X::Scope<Layer> overlay) {
    m_layers.emplace_back(std::move(overlay));
}

void LayerStack::PopLayer(Layer* layer) {
    auto it = std::find_if(m_layers.begin(), m_layers.begin() + m_layerCnt, [layer](const auto& ptr) {
        return ptr.get() == layer;
    });
    if (it != m_layers.begin() + m_layerCnt) {
        layer->OnDetach();
        m_layers.erase(it);
        --m_layerCnt;
    }
}

void LayerStack::PopOverlay(Layer* overlay) {
    auto it = std::find_if(m_layers.begin() + m_layerCnt, m_layers.end(), [overlay](const auto& ptr) {
        return ptr.get() == overlay;
    });
    if (it != m_layers.end()) {
        overlay->OnDetach();
        m_layers.erase(it);
    }
}
