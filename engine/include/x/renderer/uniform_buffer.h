//
// Created by dingrui on 3/7/26.
//

#pragma once

#include "x/core/base.h"

/**
 * OpenGL老版API是glUniformMatrix4fv给shader传uniform变量 缺点是每个shader着色程序都要传 shader之间彼此不共享
 * 内存数据往显存传是非常消耗性能的操作
 * 所以为了提升性能 新版OpenGL在显存开辟了UBO(uniform buffer object)用来当作常量内存区域 挂载到全局的binding slot上
 * 供所有的shader可以共享访问 只要在shader中使用对应的slot就行
 * 约定俗成
 *   - binding point 0 -> Camera Buffer
 *   - binding point 1 -> Light Buffer
 *   - binding point 2 -> Material Buffer
 */
class UniformBuffer {
public:
    virtual ~UniformBuffer() = default;
    virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;

    static X::Ref<UniformBuffer> Create(uint32_t size, uint32_t binding);
};
