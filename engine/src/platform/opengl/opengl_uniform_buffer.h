//
// Created by dingrui on 3/7/26.
//

#pragma once

#include "x/renderer/uniform_buffer.h"

// OpenGL的UBO
class OpenGLUniformBuffer : public UniformBuffer
{
public:
    /**
     * 创建一个GPU显存上的UBO用来灌uniform变量 好处是不用给每个shader都通过glUniformMatrix4fv传递uniform变量
     * 现在直接用一个UBO 所有shader都可以共享这个常量缓冲区 约定
     *   - binding=0放Camera
     *   - binding=1放Lights
     *   - binding=2放Renderer Data
     *   - binding=3放Material
     * @param size 在GPU显存上开辟的空间大小 多少字节
     * @param binding binding的槽位 shader着色器里面根据binding去拿uniform变量
     */
    OpenGLUniformBuffer(uint32_t size, uint32_t binding);
    ~OpenGLUniformBuffer() override;
    void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;

private:
    uint32_t m_rendererID{0};
};
