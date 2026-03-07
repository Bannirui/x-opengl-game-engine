//
// Created by dingrui on 3/7/26.
//

#pragma once

#include "x/renderer/uniform_buffer.h"

class OpenGLUniformBuffer : public UniformBuffer
{
public:
    OpenGLUniformBuffer(uint32_t size, uint32_t binding);
    ~OpenGLUniformBuffer() override;
    void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;

private:
    uint32_t m_rendererID{0};
};
