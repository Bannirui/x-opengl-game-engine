//
// Created by rui ding on 2026/6/1.
//

#include "x/renderer/render_graph.h"

#include <glad/glad.h>

static GLenum FormatToGLInternal(RenderGraphTextureFormat format) {
    switch (format) {
        case RenderGraphTextureFormat::RGBA8:
            return GL_RGBA8;
        case RenderGraphTextureFormat::RGBA16F:
            return GL_RGBA16F;
        case RenderGraphTextureFormat::R32F:
            return GL_R32F;
        case RenderGraphTextureFormat::RG16F:
            return GL_RG16F;
        case RenderGraphTextureFormat::Depth24:
            return GL_DEPTH_COMPONENT24;
    }
    return GL_RGBA8;
}

static GLenum FormatToGL(RenderGraphTextureFormat format) {
    switch (format) {
        case RenderGraphTextureFormat::RGBA8:
            return GL_RGBA;
        case RenderGraphTextureFormat::RGBA16F:
            return GL_RGBA;
        case RenderGraphTextureFormat::R32F:
            return GL_RED;
        case RenderGraphTextureFormat::RG16F:
            return GL_RG;
        case RenderGraphTextureFormat::Depth24:
            return GL_DEPTH_COMPONENT;
    }
    return GL_RGBA;
}

static GLenum FormatToGLType(RenderGraphTextureFormat format) {
    switch (format) {
        case RenderGraphTextureFormat::RGBA8:
            return GL_UNSIGNED_BYTE;
        case RenderGraphTextureFormat::RGBA16F:
            return GL_FLOAT;
        case RenderGraphTextureFormat::R32F:
            return GL_FLOAT;
        case RenderGraphTextureFormat::RG16F:
            return GL_FLOAT;
        case RenderGraphTextureFormat::Depth24:
            return GL_FLOAT;
    }
    return GL_UNSIGNED_BYTE;
}

RenderGraph::~RenderGraph() {
    for (auto& [name, res] : m_resources) {
        if (!res.External && res.RendererID) {
            glDeleteTextures(1, &res.RendererID);
        }
    }
}

uint32_t RenderGraph::CreateTexture(const std::string& name, RenderGraphTextureFormat format, uint32_t width,
                                    uint32_t height) {
    RenderGraphResource res;
    res.Type = RenderGraphResourceType::Texture2D;
    res.Format = format;
    res.Width = width;
    res.Height = height;
    res.External = false;
    allocateResource(name, res);
    return res.RendererID;
}

uint32_t RenderGraph::GetResourceID(const std::string& name) const {
    auto it = m_resources.find(name);
    if (it != m_resources.end()) return it->second.RendererID;
    return 0;
}

void RenderGraph::SetExternalResource(const std::string& name, uint32_t rendererID, RenderGraphResourceType type) {
    RenderGraphResource res;
    res.Type = type;
    res.RendererID = rendererID;
    res.External = true;
    m_resources[name] = res;
}

void RenderGraph::AddPass(const X::Ref<RenderPass>& pass) {
    m_passes.push_back(pass);
}

void RenderGraph::Compile() {
    // Let each pass declare its resources
    for (auto& pass : m_passes) {
        pass->Setup();
    }
    // Resources are allocated lazily in CreateTexture / allocateResource
}

void RenderGraph::Execute() {
    for (auto& pass : m_passes) {
        X_PROFILE_SCOPE(pass->GetName().c_str());
        pass->Execute();
    }
}

void RenderGraph::Reset() {
    m_passes.clear();
    m_resources.clear();
    m_viewportWidth = 1280;
    m_viewportHeight = 720;
}

void RenderGraph::allocateResource(const std::string& name, const RenderGraphResource& res) {
    auto it = m_resources.find(name);
    if (it != m_resources.end()) {
        if (it->second.RendererID != 0) {
            glDeleteTextures(1, &it->second.RendererID);
        }
        it->second = res;
    } else {
        m_resources[name] = res;
    }

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, FormatToGLInternal(res.Format), res.Width, res.Height, 0, FormatToGL(res.Format),
                 FormatToGLType(res.Format), nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_resources[name].RendererID = tex;
}
