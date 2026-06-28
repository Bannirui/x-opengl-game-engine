//
// Created by rui ding on 2026/6/1.
//

#pragma once

#include "x/core/base.h"

#include <cstdint>
#include <string>

class TextureCube {
public:
    static Ref<TextureCube> Create(uint32_t size, bool hdr = false);
    static Ref<TextureCube> CreateFromEquirectangular(const std::string& hdrPath);

    void Bind(uint32_t slot = 0) const;
    void Unbind() const;
    uint32_t GetRendererID() const { return m_rendererID; }
    uint32_t GetSize() const { return m_size; }

    TextureCube() = default;

public:

    uint32_t m_rendererID = 0;
    uint32_t m_size = 0;
    bool m_hdr = false;
};
