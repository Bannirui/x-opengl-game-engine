//
// Created by rui ding on 2026/6/1.
//

#pragma once

#include "x/core/base.h"

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

enum class RenderGraphResourceType {
    Texture2D,
    TextureCube,
    Framebuffer,
};

enum class RenderGraphTextureFormat {
    RGBA8,
    RGBA16F,
    R32F,
    RG16F,
    Depth24,
};

struct RenderGraphResource {
    RenderGraphResourceType Type;
    RenderGraphTextureFormat Format;
    uint32_t Width = 0;
    uint32_t Height = 0;
    uint32_t RendererID = 0;  // GPU handle
    bool External = false;
};

class RenderPass {
public:
    explicit RenderPass(const std::string& name) : m_name(name) {}
    virtual ~RenderPass() = default;

    const std::string& GetName() const { return m_name; }

    virtual void Setup() {}
    virtual void Execute() = 0;

    // Declare what this pass reads and writes (filled after Setup)
    const std::vector<std::string>& GetReads() const { return m_reads; }
    const std::vector<std::string>& GetWrites() const { return m_writes; }

protected:
    void ReadResource(const std::string& name) { m_reads.push_back(name); }
    void WriteResource(const std::string& name) { m_writes.push_back(name); }

private:
    std::string m_name;
    std::vector<std::string> m_reads;
    std::vector<std::string> m_writes;
};

class RenderGraph {
public:
    RenderGraph() = default;
    ~RenderGraph();

    // Create or get a managed resource
    uint32_t CreateTexture(const std::string& name, RenderGraphTextureFormat format, uint32_t width, uint32_t height);

    uint32_t GetResourceID(const std::string& name) const;
    void SetExternalResource(const std::string& name, uint32_t rendererID, RenderGraphResourceType type);

    // Pass management
    void AddPass(const X::Ref<RenderPass>& pass);
    void Compile();
    void Execute();
    void Reset();

    uint32_t GetViewportWidth() const { return m_viewportWidth; }
    uint32_t GetViewportHeight() const { return m_viewportHeight; }
    void SetViewportSize(uint32_t w, uint32_t h) {
        m_viewportWidth = w;
        m_viewportHeight = h;
    }

private:
    void allocateResource(const std::string& name, const RenderGraphResource& res);

    std::unordered_map<std::string, RenderGraphResource> m_resources;
    std::vector<X::Ref<RenderPass>> m_passes;
    uint32_t m_viewportWidth = 1280;
    uint32_t m_viewportHeight = 720;
};
