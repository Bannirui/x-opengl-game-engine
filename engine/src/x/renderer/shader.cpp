//
// Created by rui ding on 2026/2/27.
//

#include "x/renderer/shader.h"

#include "platform/opengl/open_gl_shader.h"
#include "x/renderer/renderer.h"
#include "x/core/x_log.h"
#include "x/core/core.h"

X::Ref<Shader> Shader::Create(const std::string &filepath)
{
    switch (Renderer::GetAPI())
    {
        case RendererAPI::API::kNone:
        {
            X_CORE_ASSERT(false, "RendererAPI::kNone is currently not supported!");
            return nullptr;
        }
        case RendererAPI::API::kOpenGL:
        {
            return X::CreateRef<OpenGLShader>(filepath);
        }
    }
    X_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}

X::Ref<Shader> Shader::Create(const std::string &name, const std::string &vertexSrc, const std::string &fragmentSrc)
{
    switch (Renderer::GetAPI())
    {
        case RendererAPI::API::kNone:
        {
            X_CORE_ASSERT(false, "RendererAPI::kNone is currently not supported!");
            return nullptr;
        }
        case RendererAPI::API::kOpenGL:
        {
            return X::CreateRef<OpenGLShader>(name, vertexSrc, fragmentSrc);
        }
    }
    X_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}

void ShaderLib::Add(const std::string &name, const X::Ref<Shader> &shader)
{
    X_CORE_ASSERT(!Exists(name), "Shader already exists!");
    m_shaders[name] = shader;
}

void ShaderLib::Add(const X::Ref<Shader> &shader)
{
    auto &name = shader->get_name();
    Add(name, shader);
}

X::Ref<Shader> ShaderLib::Load(const std::string &filepath)
{
    auto shader = Shader::Create(filepath);
    Add(shader);
    return shader;
}

X::Ref<Shader> ShaderLib::Load(const std::string &name, const std::string &filepath)
{
    auto shader = Shader::Create(filepath);
    Add(name, shader);
    return shader;
}

X::Ref<Shader> ShaderLib::Get(const std::string &name)
{
    X_CORE_ASSERT(Exists(name), "Shader not found!");
    return m_shaders[name];
}

bool ShaderLib::Exists(const std::string &name) const
{
    return m_shaders.find(name) != m_shaders.end();
}
