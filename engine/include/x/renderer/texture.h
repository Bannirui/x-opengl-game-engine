//
// Created by dingrui on 2/28/26.
//

#pragma once

#include "x/core/base.h"

#include <cstdint>

/**
 * 相比较于给很多顶点设置颜色以达到给物体生成纹理 用一个2D图片作为物体纹理 这样的方式代价很低
 * Texture的作用就是封装2D图片给物体设置纹理
 * 当然 理论上也存在1D和3D的纹理对象
 * 纹理坐标uv坐标左下角是(0,0)它的范围是[0...1] 用uv坐标在纹理对象上找贴图颜色的过程叫采样sample
 */
class Texture {
public:
    virtual ~Texture() = default;

    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;
    virtual uint32_t GetRendererID() const = 0;
    virtual const std::string& GetPath() const = 0;
    /**
     * 把内存上的数据传到显存上
     * @param data 内存上的数据
     * @param size 多大 字节
     */
    virtual void SetData(void* data, uint32_t size) = 0;

    // texture uint
    virtual void Bind(uint32_t slot = 0) const = 0;
    virtual bool IsLoaded() = 0;

    virtual bool operator==(const Texture& other) const = 0;

protected:
    Texture() = default;
};

class Texture2D : public Texture {
public:
    /**
     * @param path 图片路径
     */
    static X::Ref<Texture2D> Create(const std::string& path);
    /**
     * 不是用磁盘上的图片 一般是先生成一个纹理对象 然后SetData传一个4Byte的RBGA到显存
     * @param width 图片宽度
     * @param height 图片高度
     */
    static X::Ref<Texture2D> Create(uint32_t width, uint32_t height);
};
