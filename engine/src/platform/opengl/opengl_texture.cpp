//
// Created by dingrui on 2/28/26.
//

#include "platform/opengl/opengl_texture.h"

#include "x/core/assert.h"
#include "x/core/base.h"
#include "x/core/log.h"

#include <glad/glad.h>

#include <stb_image.h>

/**
 * 创建纹理对象
 * @param path 图片路径
 */
OpenGLTexture2D::OpenGLTexture2D(const std::string& path) : m_path(path) {
    X_PROFILE_FUNCTION();
    int width, height, channels;
    // OpenGL的y坐标和stb的y坐标定义方向不一样 OpenGL的0在底部 图片的0在顶部 所以要让stb翻转一下 让OpenGL拿到正确方向
    stbi_set_flip_vertically_on_load(true);
    stbi_uc* data{nullptr};
    {
        X_PROFILE_SCOPE("stbi_load - OpenGLTexture2D::OpenGLTexture2D(const std::string&)");
        data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    }
    X_CORE_ASSERT(data, "Failed to load image: {}", path);
    m_isLoaded = true;
    m_width = width;
    m_height = height;

    GLenum internalFormat = 0, dataFormat = 0;
    if (channels == 4) {
        internalFormat = GL_RGBA8;
        dataFormat = GL_RGBA;
    } else if (channels == 3) {
        internalFormat = GL_RGB8;
        dataFormat = GL_RGB;
    }
    // 生成纹理对象
    glGenTextures(1, &m_rendererId);
    // 激活纹理对象 也可以不用显式激活纹理单元 因为下面的BindTexture这个API会默认自动激活纹理单元0号
    glActiveTexture(GL_TEXTURE0);
    // 绑定纹理对象 类型是2d
    glBindTexture(GL_TEXTURE_2D, m_rendererId);

    // 分配cpu内存
    m_internalFormat = internalFormat;
    m_dataFormat = dataFormat;
    X_CORE_ASSERT(internalFormat & dataFormat, "Format not supported!");
    // 用图片生成一个纹理对象贴图 mipmap level设置0代表的是没有给纹理对象设置mipmap 可以在生成纹理对象后再给它设置mipmap
    glTexImage2D(GL_TEXTURE_2D,  // 纹理对象在上面已经绑定了OpenGL的2d插槽 现在要配置2d插槽就是在配置纹理对象
                 0,  // mipmap的level 不用mipmap就设置0
                 internalFormat,  // 告诉OpenGL图片的格式
                 m_width,  // 图片宽度
                 m_height,  // 图片高度
                 0,  // 为了兼容早期设计导致的 直接设置成0就行
                 dataFormat,  // 图片数据格式
                 GL_UNSIGNED_BYTE,
                 data  // 实际的图片
    );
    // 最佳的工程实践 在创建好了纹理对象也设置好了mipmap后就把图片的空间释放掉
    stbi_image_free(data);

    // 设置纹理对象参数
    /**
     * 电脑屏幕的像素和纹理像素的映射问题 它们几乎不会严格完整映射
     *   - 图片放大时 一个texel要覆盖很多pixel GPU面临的问题是中间色怎么办
     *     如果不用filtering GPU会直接复制最近的texel会导致马赛克
     *   - 图片缩小时 一个pixel对应成千上万个texel 这个时候GPU面临的问题更大 选哪个texel显示
     *     如果随便取就近的一个texel 会导致严重的走样
     *       - 闪烁
     *       - 摩尔纹
     *       - 抖动
     *       - 远处噪声
     */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    /**
     * 纹理的坐标是0到1 起点是左下方的xy坐标系
     * 指定的贴图坐标是(0,0)到(1,1) 要是超出了这个约定范围怎么办
     *   - S对应U x横向
     *   - T对应V y纵向
     * 默认的策略是repeat
     * OpenGL的策略
     *   - repeat 循环
     *   - mirror repeat 镜像循环
     *   - clamp to edge 超出后用边缘的像素
     *     - <0 用最左边的像素
     *     - >1 用最右边的像素
     *   - clamp to border 越界后不采样纹理 直接用borderColor 就是要先给shader设置一个borderColor一旦越界就用它
     */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // 让OpenGL状态机上的2D纹理插槽撤掉当前纹理对象 防止后面误操作当前的纹理对象
    glBindTexture(GL_TEXTURE_2D, 0);
}

OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height) : m_width(width), m_height(height) {
    X_PROFILE_FUNCTION();
    m_internalFormat = GL_RGBA8;
    m_dataFormat = GL_RGBA;

    glGenTextures(1, &m_rendererId);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_rendererId);

    glTexImage2D(GL_TEXTURE_2D, 0, m_internalFormat, m_width, m_height, 0, m_dataFormat, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);
}

OpenGLTexture2D::~OpenGLTexture2D() {
    if (m_rendererId) {
        glDeleteTextures(1, &m_rendererId);
        m_rendererId = 0;
    }
}

void OpenGLTexture2D::SetData(void* data, uint32_t size) {
    X_PROFILE_FUNCTION();
    uint32_t bpp = m_dataFormat == GL_RGBA ? 4 : 3;
    X_CORE_ASSERT(size == m_width * m_height * bpp, "Incorrect size!");
    // 绑定纹理对象
    glBindTexture(GL_TEXTURE_2D, m_rendererId);
    // 更新数据
    glTexSubImage2D(GL_TEXTURE_2D,
                    0,  // mip level
                    0, 0,  // offset
                    m_width, m_height, m_dataFormat, GL_UNSIGNED_BYTE, data);
    // 解绑
    glBindTexture(GL_TEXTURE_2D, 0);
}

/**
 * 激活贴图的纹理单元
 * @param slot 激活贴图缓冲区的哪个贴图的纹理单元
 *             在引擎上会开辟内存缓冲16个贴图 在执行渲染的时候会全部激活对应的纹理单元
 *             shader用哪个会通过vertex attribute方式告诉贴图脚标
 */
void OpenGLTexture2D::Bind(uint32_t slot) const {
    X_PROFILE_FUNCTION();
    // OpenGL是状态机 不知道当前状态机的纹理单元 所以要先切换纹理单元 然后绑定纹理对象
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_rendererId);
}
