//
// Created by dingrui on 2/28/26.
//

#include "platform/opengl/open_gl_shader.h"

#include <glad/glad.h>

#include <GL/glext.h>
#include <glm/gtc/type_ptr.inl>
#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#include "x/core/base.h"
#include "x/core/timer.h"
#include "x/core/x_log.h"

namespace Util
{
    // shader源码用#type vertex跟#type fragment区分
    static GLenum shaderTypeFromString(const std::string& type)
    {
        if (type == "vertex")
        {
            return GL_VERTEX_SHADER;
        }
        if (type == "fragment" || type == "pixel")
        {
            return GL_FRAGMENT_SHADER;
        }
        X_CORE_ASSERT(false, "Unknown shader type");
        return 0;
    }

    static shaderc_shader_kind GLShaderStageToShaderC(GLenum stage)
    {
        switch (stage)
        {
            case GL_VERTEX_SHADER:
            {
                return shaderc_glsl_vertex_shader;
            }
            case GL_FRAGMENT_SHADER:
            {
                return shaderc_glsl_fragment_shader;
            }
        }
        X_CORE_ASSERT(false);
        return (shaderc_shader_kind)0;
    }

    static const char* GLShaderStageToString(GLenum stage)
    {
        switch (stage)
        {
            case GL_VERTEX_SHADER:
            {
                return "GL_VERTEX_SHADER";
            }
            case GL_FRAGMENT_SHADER:
            {
                return "GL_FRAGMENT_SHADER";
            }
        }
        X_CORE_ASSERT(false);
        return nullptr;
    }

    static const char* GetCacheDirectory()
    {
        return "asset/shader";
    }

    static void CreateCacheDirectoryIfNeeded()
    {
        std::string cacheDirectory = GetCacheDirectory();
        if (!std::filesystem::exists(cacheDirectory))
        {
            std::filesystem::create_directories(cacheDirectory);
        }
    }

    static const char* GLShaderStageCachedOpenGFileExtension(uint32_t stage)
    {
        switch (stage)
        {
            case GL_VERTEX_SHADER:
            {
                return ".cached_opengl.vert";
            }
            case GL_FRAGMENT_SHADER:
            {
                return ".cached_opengl.frag";
            }
        }
        X_CORE_ASSERT(false, "Unknown shader stage");
        return "";
    }

    static const char* GLShaderStageCachedVulkanFilExtension(uint32_t stage)
    {
        switch (stage)
        {
            case GL_VERTEX_SHADER:
                return ".cached_vulkan.vert";
            case GL_FRAGMENT_SHADER:
                return ".cached_vulkan.frag";
        }
        X_CORE_ASSERT(false);
        return "";
    }
}  // namespace Util

static std::string replaceVersion(std::string source, const std::string& versionStr)
{
    const char* versionToken = "#version";
    size_t      versionPos   = source.find(versionToken, 0);
    while (versionPos != std::string::npos)
    {
        size_t eol = source.find_first_of("\r\n", versionPos);
        source.replace(versionPos, eol - versionPos, versionStr);
        size_t nextLinePos = source.find_first_not_of("\r\n", eol);
        versionPos         = source.find(versionToken, nextLinePos);
    }
    return source;
}

OpenGLShader::OpenGLShader(const std::string& filepath) : m_filePath(filepath)
{
    X_PROFILE_FUNCTION();
    Util::CreateCacheDirectoryIfNeeded();
    std::string source        = readFile(filepath);
    auto        shaderSources = preProcess(source);
    {
        Timer timer;
        compileOrGetVulkanBinaries(shaderSources);
        compileOrGetOpenGLBinaries();
        creatProgram();
        X_CORE_INFO("Shader creation took {} ms", timer.ElapsedMillis());
    }
    // 文件名
    auto lastSlash = filepath.find_last_of("/\\");
    lastSlash      = lastSlash == std::string::npos ? 0 : lastSlash + 1;
    auto lastDot   = filepath.rfind('.');
    auto count     = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
    m_name         = filepath.substr(lastSlash, count);
}

OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string fragmentSrc)
    : m_name(name)
{
    X_PROFILE_FUNCTION();
    std::unordered_map<GLenum, std::string> shaderSources;
    shaderSources[GL_VERTEX_SHADER]   = vertexSrc;
    shaderSources[GL_FRAGMENT_SHADER] = fragmentSrc;
    compileOrGetVulkanBinaries(shaderSources);
    compileOrGetOpenGLBinaries();
    creatProgram();
}

OpenGLShader::~OpenGLShader()
{
    X_PROFILE_FUNCTION();
    if (m_rendererId > 0)
    {
        glDeleteProgram(m_rendererId);
    }
}

void OpenGLShader::Bind() const
{
    X_PROFILE_FUNCTION();
    glUseProgram(m_rendererId);
}

void OpenGLShader::Unbind() const
{
    X_PROFILE_FUNCTION();
    glUseProgram(0);
}

void OpenGLShader::SetInt(const std::string& name, int value)
{
    X_PROFILE_FUNCTION();
    uploadUniformInt(name, value);
}

void OpenGLShader::SetIntArray(const std::string& name, int* values, int count)
{
    X_PROFILE_FUNCTION();
    uploadUniformIntArray(name, values, count);
}

void OpenGLShader::SetFloat(const std::string& name, float value)
{
    X_PROFILE_FUNCTION();
    uploadUniformFloat(name, value);
}

void OpenGLShader::SetFloat2(const std::string& name, const glm::vec2& value)
{
    X_PROFILE_FUNCTION();
    uploadUniformFloat2(name, value);
}

void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
{
    X_PROFILE_FUNCTION();
    uploadUniformFloat3(name, value);
}

void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
{
    X_PROFILE_FUNCTION();
    uploadUniformFloat4(name, value);
}

void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
{
    X_PROFILE_FUNCTION();
    uploadUniformMat4(name, value);
}

void OpenGLShader::uploadUniformInt(const std::string& name, int value)
{
    GLint location = glGetUniformLocation(m_rendererId, name.c_str());
    glUniform1i(location, value);
}

void OpenGLShader::uploadUniformIntArray(const std::string& name, int* values, uint32_t count)
{
    GLint location = glGetUniformLocation(m_rendererId, name.c_str());
    glUniform1iv(location, count, values);
}

void OpenGLShader::uploadUniformFloat(const std::string& name, float value)
{
    GLint location = glGetUniformLocation(m_rendererId, name.c_str());
    glUniform1f(location, value);
}

void OpenGLShader::uploadUniformFloat2(const std::string& name, const glm::vec2& value)
{
    GLint location = glGetUniformLocation(m_rendererId, name.c_str());
    glUniform2f(location, value.x, value.y);
}

void OpenGLShader::uploadUniformFloat3(const std::string& name, const glm::vec3& value)
{
    GLint location = glGetUniformLocation(m_rendererId, name.c_str());
    glUniform3f(location, value.x, value.y, value.z);
}

void OpenGLShader::uploadUniformFloat4(const std::string& name, const glm::vec4& value)
{
    GLint location = glGetUniformLocation(m_rendererId, name.c_str());
    glUniform4f(location, value.x, value.y, value.z, value.w);
}

void OpenGLShader::uploadUniformMat3(const std::string& name, const glm::mat3& matrix)
{
    GLint location = glGetUniformLocation(m_rendererId, name.c_str());
    glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

void OpenGLShader::uploadUniformMat4(const std::string& name, const glm::mat4& matrix)
{
    GLint location = glGetUniformLocation(m_rendererId, name.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

std::string OpenGLShader::readFile(const std::string& filepath)
{
    X_PROFILE_FUNCTION();
    std::string   ret;
    std::ifstream in(filepath, std::ios::in | std::ios::binary);  // ifstream closes itself due to RAII
    X_CORE_ASSERT(in, "Could not open file");
    in.seekg(0, std::ios::end);
    size_t size = in.tellg();
    X_CORE_ASSERT(size != 1, "Could not read from file");
    ret.resize(size);
    in.seekg(0, std::ios::beg);
    in.read(&ret[0], ret.size());
    return ret;
}

std::unordered_map<GLenum, std::string> OpenGLShader::preProcess(const std::string& glslSrc)
{
    X_PROFILE_FUNCTION();
    std::string modifiedSource = glslSrc;
    // 动态替换#version占位符
    if (modifiedSource.find("#version") != std::string::npos)
    {
        X_CORE_INFO("shader using OpenGL:{}", X_GL_VERSION_CORE);
        modifiedSource = replaceVersion(modifiedSource, X_GL_VERSION_CORE);
    }
    std::unordered_map<GLenum, std::string> shaderSources;
    const char*                             typeToken       = "#type";  // vertex跟frag的区分用#type vertex跟#type frag
    size_t                                  typeTokenLength = strlen(typeToken);
    size_t                                  pos             = modifiedSource.find(typeToken, 0);  // 找到第一个#type标识
    while (pos != std::string::npos)
    {
        size_t eol = modifiedSource.find_first_of("\r\n", pos);  // 定位位#type那一行最后 中间就是#type的内容
        X_CORE_ASSERT(eol != std::string::npos, "Syntax error");
        size_t      begin = pos + typeTokenLength + 1;
        std::string type  = modifiedSource.substr(begin, eol - begin);  // vertex or frag
        X_CORE_ASSERT(Util::shaderTypeFromString(type), "Invalid shader type specified");
        size_t nextLinePos = modifiedSource.find_first_not_of("\r\n", eol);
        X_CORE_ASSERT(nextLinePos != std::string::npos, "Syntax error");
        pos                                             = modifiedSource.find(typeToken, nextLinePos);
        shaderSources[Util::shaderTypeFromString(type)] = (pos == std::string::npos)
                                                              ? modifiedSource.substr(nextLinePos)
                                                              : modifiedSource.substr(nextLinePos, pos - nextLinePos);
    }
    return shaderSources;
}

void OpenGLShader::compileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources)
{
    // todo
    // GLuint                  program = glCreateProgram();
    shaderc::Compiler       compiler;
    shaderc::CompileOptions options;
    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
    const bool optimize = true;
    if (optimize)
    {
        options.SetOptimizationLevel(shaderc_optimization_level_performance);
    }
    std::filesystem::path cacheDirectory = Util::GetCacheDirectory();
    auto&                 shaderData     = m_vulkanSPIRV;
    shaderData.clear();
    for (auto&& [stage, source] : shaderSources)
    {
        std::filesystem::path shaderFilePath = m_filePath;
        std::filesystem::path cachedPath =
            cacheDirectory / (shaderFilePath.filename().string() + Util::GLShaderStageCachedVulkanFilExtension(stage));
        std::ifstream in(source, std::ios::in | std::ios::binary);
        if (in.is_open())
        {
            in.seekg(0, std::ios::end);
            auto size = in.tellg();
            in.seekg(0, std::ios::beg);
            auto& data = shaderData[stage];
            data.resize(size / sizeof(uint32_t));
            in.read((char*)data.data(), size);
        }
        else
        {
            shaderc::SpvCompilationResult module =
                compiler.CompileGlslToSpv(source, Util::GLShaderStageToShaderC(stage), m_filePath.c_str(), options);
            if (module.GetCompilationStatus() != shaderc_compilation_status_success)
            {
                X_CORE_ERROR(module.GetErrorMessage());
                X_CORE_ASSERT(false);
            }
            shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());
            std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
            if (out.is_open())
            {
                auto& data = shaderData[stage];
                out.write((char*)data.data(), data.size() * sizeof(uint32_t));
                out.flush();
                out.close();
            }
        }
    }
    for (auto& [stage, data] : shaderData)
    {
        reflect(stage, data);
    }
}

void OpenGLShader::compileOrGetOpenGLBinaries()
{
    auto&                   shaderData = m_openGLSPIRV;
    shaderc::Compiler       compiler;
    shaderc::CompileOptions options;
    options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
    const bool optimize = true;
    if (optimize)
    {
        options.SetOptimizationLevel(shaderc_optimization_level_performance);
    }
    std::filesystem::path cachedDirectory = Util::GetCacheDirectory();
    shaderData.clear();
    m_openGLSourceCode.clear();
    for (auto&& [stage, spirv] : m_vulkanSPIRV)
    {
        std::filesystem::path shaderFilePath = m_filePath;
        std::filesystem::path cachedPath =
            cachedDirectory / (shaderFilePath.filename().string() + Util::GLShaderStageCachedOpenGFileExtension(stage));
        std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
        if (in.is_open())
        {
            in.seekg(0, std::ios::end);
            auto size = in.tellg();
            in.seekg(0, std::ios::beg);
            auto& data = shaderData[stage];
            data.resize(size / sizeof(uint32_t));
            in.read((char*)data.data(), size);
        }
        else
        {
            spirv_cross::CompilerGLSL glslCompiler(spirv);
            m_openGLSourceCode[stage]            = glslCompiler.compile();
            auto&                         source = m_openGLSourceCode[stage];
            shaderc::SpvCompilationResult module =
                compiler.CompileGlslToSpv(source, Util::GLShaderStageToShaderC(stage), m_filePath.c_str());
            if (module.GetCompilationStatus() != shaderc_compilation_status_success)
            {
                X_CORE_ERROR(module.GetErrorMessage());
                X_CORE_ASSERT(false);
            }
            shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());
            std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
            if (out.is_open())
            {
                auto& data = shaderData[stage];
                out.write((char*)data.data(), data.size() * sizeof(uint32_t));
                out.flush();
                out.close();
            }
        }
    }
}

void OpenGLShader::creatProgram()
{
    GLuint              program = glCreateProgram();
    std::vector<GLuint> shaderIDs;
    for (auto&& [stage, spirv] : m_openGLSPIRV)
    {
        GLuint shaderID = shaderIDs.emplace_back(glCreateShader(stage));
        glShaderBinary(1, &shaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv.data(), spirv.size() * sizeof(uint32_t));
        glSpecializeShader(shaderID, "main", 0, nullptr, nullptr);
        glAttachShader(program, shaderID);
    }
    glLinkProgram(program);
    GLint isLinked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());
        X_CORE_ERROR("Shader linking failed ({}):\n{}", m_filePath, infoLog.data());
        glDeleteProgram(program);
        for (auto id : shaderIDs)
        {
            glDeleteShader(id);
        }
    }
    for (auto id : shaderIDs)
    {
        glDetachShader(program, id);
        glDeleteShader(id);
    }
    m_rendererId = program;
}

void OpenGLShader::reflect(GLenum stage, const std::vector<uint32_t>& shaderData)
{
    spirv_cross::Compiler        compiler(shaderData);
    spirv_cross::ShaderResources resources = compiler.get_shader_resources();
    X_CORE_TRACE("OpenGLShader::reflect - {} {}", Util::GLShaderStageToString(stage), m_filePath);
    X_CORE_TRACE("  {} uniform buffers", resources.uniform_buffers.size());
    X_CORE_TRACE("  {} resources", resources.sampled_images.size());
    X_CORE_TRACE("Uniform buffers:");
    for (const auto& resource : resources.uniform_buffers)
    {
        const auto& bufferType  = compiler.get_type(resource.base_type_id);
        uint32_t    bufferSize  = compiler.get_declared_struct_size(bufferType);
        uint32_t    binding     = compiler.get_decoration(resource.id, spv::DecorationBinding);
        int         memberCount = bufferType.member_types.size();
        X_CORE_TRACE("  {}", resource.name);
        X_CORE_TRACE("    Size = {}", bufferSize);
        X_CORE_TRACE("    Binding = {}", binding);
        X_CORE_TRACE("    Members = {}", memberCount);
    }
}