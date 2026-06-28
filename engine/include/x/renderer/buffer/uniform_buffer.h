//
// Created by dingrui on 3/7/26.
//

#pragma once

#include "x/core/base.h"

/**
 * UBO
 * Uniform Buffer Object
 * OpenGL在显存开辟常量内存区域 挂载到全局binding slot
 * 所有shader共享访问 减少CPU到GPU数据传输
 * C++端
 *   - UniformBuffer::Create(binding)
 * GLSL端
 *   - layout(binding=N)必须一一对应
 * 枚举每个UBO在shader里面的binding
 */
namespace UBOBinding {
    constexpr uint32_t Camera = 0;  // 相机ViewProjection矩阵 所有shader共享
    constexpr uint32_t Model = 1;  // 模型世界矩阵 每物体更新
    constexpr uint32_t Light = 2;  // 方向光参数 每帧更新
    constexpr uint32_t PBRSettings = 3;  // PBR 摄像机位置+曝光度
    constexpr uint32_t LightSpace = 4;  // 光源空间VP矩阵 阴影映射
    constexpr uint32_t CSMData = 5;  // CSM级联分割数据
}  // namespace UBOBinding

class UniformBuffer {
public:
    virtual ~UniformBuffer() = default;
    /**
     * 把CPU内存上的数据灌给UBO 也就是把数据从CPU内存写到GPU显存
     * [x...offset...y)比如内存地址x上 从offset开始读 offset到y总共n个字节的数据
     * 把这些数据读出来写给显存
     * @param data CPU内存上的数据地址
     * @param size CPU内存上多大的数据 多少个字节
     * @param offset 从这个CPU内存地址什么地方开始读
     */
    virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;
    virtual void Bind() const = 0;

    static Ref<UniformBuffer> Create(uint32_t size, uint32_t binding);
};
