//
// Created by dingrui on 2/28/26.
//

#include "platform/opengl/opengl_shader.h"

#include "x/core/base.h"
#include "x/core/timer.h"
#include "x/core/x_log.h"

#include <glad/glad.h>

#include <glm/gtc/type_ptr.inl>

#include <shaderc/shaderc.hpp>

#include <spirv_cross.hpp>

namespace Util {
    static GLenum shaderTypeFromString(const std::string& type) {
        if (type == "vertex") {
            return GL_VERTEX_SHADER;
        }
        if (type == "fragment" || type == "pixel") {
            return GL_FRAGMENT_SHADER;
        }
        X_CORE_ASSERT(false, "Unknown shader type");
        return 0;
    }

#ifdef glSpecializeShaderARB
    static shaderc_shader_kind GLShaderStageToShaderC(GLenum stage) {
        switch (stage) {
            case GL_VERTEX_SHADER:
                return shaderc_glsl_vertex_shader;
            case GL_FRAGMENT_SHADER:
                return shaderc_glsl_fragment_shader;
        }
        X_CORE_ASSERT(false);
        return (shaderc_shader_kind)0;
    }
#endif

    static const char* GLShaderStageToString(GLenum stage) {
        switch (stage) {
            case GL_VERTEX_SHADER:
                return "GL_VERTEX_SHADER";
            case GL_FRAGMENT_SHADER:
                return "GL_FRAGMENT_SHADER";
        }
        X_CORE_ASSERT(false);
        return nullptr;
    }

    static const char* GetCacheDirectory() {
        return "asset/shader";
    }

    static void CreateCacheDirectoryIfNeeded() {
        std::string cacheDirectory = GetCacheDirectory();
        if (!std::filesystem::exists(cacheDirectory)) {
            std::filesystem::create_directories(cacheDirectory);
        }
    }

#ifdef glSpecializeShaderARB
    static const char* GLShaderStageCachedFileExtension(GLenum stage) {
        switch (stage) {
            case GL_VERTEX_SHADER:
                return ".cached_spirv.vert";
            case GL_FRAGMENT_SHADER:
                return ".cached_spirv.frag";
        }
        X_CORE_ASSERT(false, "Unknown shader stage");
        return "";
    }
#endif
}  // namespace Util

static std::string replaceVersion(std::string source, const std::string& versionStr) {
    const char* versionToken = "#version";
    size_t versionPos = source.find(versionToken, 0);
    while (versionPos != std::string::npos) {
        size_t eol = source.find_first_of("\r\n", versionPos);
        source.replace(versionPos, eol - versionPos, versionStr);
        size_t nextLinePos = source.find_first_not_of("\r\n", eol);
        versionPos = source.find(versionToken, nextLinePos);
    }
    return source;
}

static std::string resolveShaderFilePath(const std::string& originalPath) {
    size_t dotGlslPos = originalPath.rfind(".glsl");
    if (dotGlslPos == std::string::npos) {
        return originalPath;
    }
    auto& info = X::GLRendererInfo::Get();
    int versionDigits = info.MajorVersion * 100 + info.MinorVersion * 10;
    std::string versionSuffix = "." + std::to_string(versionDigits);
    std::string candidate = originalPath.substr(0, dotGlslPos) + versionSuffix + ".glsl";
    if (std::filesystem::exists(candidate)) {
        X_CORE_INFO("shader resolved to version-specific file: {}", candidate);
        return candidate;
    }
    return originalPath;
}

/**
 * 看看在GLSL源码路径里面有没有指定版本号
 * @param filepath GLSL的源码路径
 */
static bool isVersionSpecificFilePath(const std::string& filepath) {
    size_t dotGlslPos = filepath.rfind(".glsl");
    if (dotGlslPos == std::string::npos || dotGlslPos < 4) {
        return false;
    }
    std::string beforeGlsl = filepath.substr(0, dotGlslPos);
    size_t lastDot = beforeGlsl.rfind('.');
    if (lastDot == std::string::npos) {
        return false;
    }
    std::string potentialVersion = beforeGlsl.substr(lastDot + 1);
    if (potentialVersion.length() != 3) {
        return false;
    }
    for (char c : potentialVersion) {
        if (c < '0' || c > '9') {
            return false;
        }
    }
    return true;
}

/**
 * 这个源码路径可能是x.glsl
 * 拿到这个路径并不是直接就用它的源码了
 *   - 1 先拼接上运行时的OpenGL版本成为类似x.330.glsl路径
 *   - 2 找不到带版本的源码再用找传进来的路径
 * 达到了根据运行时OpenGL版本动态适配GLSL语法的效果
 * @param filepath GLSL源码路径
 */
OpenGLShader::OpenGLShader(const std::string& filepath) {
    X_PROFILE_FUNCTION();
    // GLSL源码路径
    m_filePath = resolveShaderFilePath(filepath);
    Util::CreateCacheDirectoryIfNeeded();
    std::string source = readFile(m_filePath);
    auto shaderSources = preProcess(source);
    {
        Timer timer;
        compileOrGetBinaries(shaderSources);
        creatProgram();
        X_CORE_INFO("Shader creation took {} ms", timer.ElapsedMillis());
    }
    auto lastSlash = filepath.find_last_of("/\\");
    lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
    auto lastDot = filepath.rfind('.');
    auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
    m_name = filepath.substr(lastSlash, count);
}

OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string fragmentSrc)
    : m_name(name) {
    X_PROFILE_FUNCTION();
    std::unordered_map<GLenum, std::string> shaderSources;
    shaderSources[GL_VERTEX_SHADER] = vertexSrc;
    shaderSources[GL_FRAGMENT_SHADER] = fragmentSrc;
    compileOrGetBinaries(shaderSources);
    creatProgram();
}

OpenGLShader::~OpenGLShader() {
    X_PROFILE_FUNCTION();
    if (m_rendererId > 0) {
        glDeleteProgram(m_rendererId);
    }
}

void OpenGLShader::Bind() const {
    X_PROFILE_FUNCTION();
    glUseProgram(m_rendererId);
}

void OpenGLShader::Unbind() const {
    X_PROFILE_FUNCTION();
    glUseProgram(0);
}

void OpenGLShader::SetInt(const std::string& name, int value) {
    X_PROFILE_FUNCTION();
    uploadUniformInt(name, value);
}

void OpenGLShader::SetIntArray(const std::string& name, int* values, int count) {
    X_PROFILE_FUNCTION();
    uploadUniformIntArray(name, values, count);
}

void OpenGLShader::SetFloat(const std::string& name, float value) {
    X_PROFILE_FUNCTION();
    uploadUniformFloat(name, value);
}

void OpenGLShader::SetFloat2(const std::string& name, const glm::vec2& value) {
    X_PROFILE_FUNCTION();
    uploadUniformFloat2(name, value);
}

void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value) {
    X_PROFILE_FUNCTION();
    uploadUniformFloat3(name, value);
}

void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value) {
    X_PROFILE_FUNCTION();
    uploadUniformFloat4(name, value);
}

void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value) {
    X_PROFILE_FUNCTION();
    uploadUniformMat4(name, value);
}

void OpenGLShader::uploadUniformInt(const std::string& name, int value) {
    GLint location = glGetUniformLocation(m_rendererId, name.c_str());
    glUniform1i(location, value);
}

void OpenGLShader::uploadUniformIntArray(const std::string& name, int* values, uint32_t count) {
    GLint location = glGetUniformLocation(m_rendererId, name.c_str());
    glUniform1iv(location, count, values);
}

void OpenGLShader::uploadUniformFloat(const std::string& name, float value) {
    GLint location = glGetUniformLocation(m_rendererId, name.c_str());
    glUniform1f(location, value);
}

void OpenGLShader::uploadUniformFloat2(const std::string& name, const glm::vec2& value) {
    GLint location = glGetUniformLocation(m_rendererId, name.c_str());
    glUniform2f(location, value.x, value.y);
}

void OpenGLShader::uploadUniformFloat3(const std::string& name, const glm::vec3& value) {
    GLint location = glGetUniformLocation(m_rendererId, name.c_str());
    glUniform3f(location, value.x, value.y, value.z);
}

void OpenGLShader::uploadUniformFloat4(const std::string& name, const glm::vec4& value) {
    GLint location = glGetUniformLocation(m_rendererId, name.c_str());
    glUniform4f(location, value.x, value.y, value.z, value.w);
}

void OpenGLShader::uploadUniformMat3(const std::string& name, const glm::mat3& matrix) {
    GLint location = glGetUniformLocation(m_rendererId, name.c_str());
    glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

void OpenGLShader::uploadUniformMat4(const std::string& name, const glm::mat4& matrix) {
    GLint location = glGetUniformLocation(m_rendererId, name.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

std::string OpenGLShader::readFile(const std::string& filepath) {
    X_PROFILE_FUNCTION();
    std::string ret;
    std::ifstream in(filepath, std::ios::in | std::ios::binary);
    X_CORE_ASSERT(in, "Could not open file");
    in.seekg(0, std::ios::end);
    size_t size = in.tellg();
    X_CORE_ASSERT(size != 1, "Could not read from file");
    ret.resize(size);
    in.seekg(0, std::ios::beg);
    in.read(&ret[0], ret.size());
    return ret;
}

std::unordered_map<GLenum, std::string> OpenGLShader::preProcess(const std::string& glslSrc) {
    X_PROFILE_FUNCTION();
    std::string modifiedSource = glslSrc;
    if (!isVersionSpecificFilePath(m_filePath) && modifiedSource.find("#version") != std::string::npos) {
        // GLSL源码里面没有指定版本号 就用备用逻辑 把源码里面的#version声明替换成运行时OpenGL的版本 然后用源码创建shader程序
        X_CORE_INFO("shader using OpenGL:{}", X::GLRendererInfo::Get().GetGLSLVersionString());
        modifiedSource = replaceVersion(modifiedSource, X::GLRendererInfo::Get().GetGLSLVersionString());
    }
    // 在GLSL源码路径里面指定了版本的不要动里面的#version声明 在编写的时候已经适配了对应的版本了
    std::unordered_map<GLenum, std::string> shaderSources;
    const char* typeToken = "#type";
    size_t typeTokenLength = strlen(typeToken);
    size_t pos = modifiedSource.find(typeToken, 0);
    while (pos != std::string::npos) {
        size_t eol = modifiedSource.find_first_of("\r\n", pos);
        X_CORE_ASSERT(eol != std::string::npos, "Syntax error");
        size_t begin = pos + typeTokenLength + 1;
        std::string type = modifiedSource.substr(begin, eol - begin);
        X_CORE_ASSERT(Util::shaderTypeFromString(type), "Invalid shader type specified");
        size_t nextLinePos = modifiedSource.find_first_not_of("\r\n", eol);
        X_CORE_ASSERT(nextLinePos != std::string::npos, "Syntax error");
        pos = modifiedSource.find(typeToken, nextLinePos);
        shaderSources[Util::shaderTypeFromString(type)] = (pos == std::string::npos)
                                                              ? modifiedSource.substr(nextLinePos)
                                                              : modifiedSource.substr(nextLinePos, pos - nextLinePos);
    }
    return shaderSources;
}

void OpenGLShader::compileOrGetBinaries(const std::unordered_map<GLenum, std::string>& shaderSources) {
    m_glslSources = shaderSources;

    if (X::GLRendererInfo::Get().ARB_gl_spirv) {
#ifdef glSpecializeShaderARB
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
        options.SetOptimizationLevel(shaderc_optimization_level_performance);

        std::filesystem::path cacheDirectory = Util::GetCacheDirectory();
        bool useCache = !m_filePath.empty();
        m_spirvBinaries.clear();
        for (auto&& [stage, source] : shaderSources) {
            if (useCache) {
                std::filesystem::path cachedPath =
                    cacheDirectory / (std::filesystem::path(m_filePath).filename().string() +
                                      Util::GLShaderStageCachedFileExtension(stage));
                std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
                if (in.is_open()) {
                    in.seekg(0, std::ios::end);
                    auto size = in.tellg();
                    in.seekg(0, std::ios::beg);
                    auto& data = m_spirvBinaries[stage];
                    data.resize(size / sizeof(uint32_t));
                    in.read((char*)data.data(), size);
                    reflect(stage, m_spirvBinaries[stage]);
                    continue;
                }
            }

            shaderc::SpvCompilationResult module =
                compiler.CompileGlslToSpv(source, Util::GLShaderStageToShaderC(stage), m_filePath.c_str(), options);
            if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
                X_CORE_ERROR(module.GetErrorMessage());
                X_CORE_ASSERT(false);
            }
            m_spirvBinaries[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

            if (useCache) {
                std::filesystem::path cachedPath =
                    cacheDirectory / (std::filesystem::path(m_filePath).filename().string() +
                                      Util::GLShaderStageCachedFileExtension(stage));
                std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
                if (out.is_open()) {
                    auto& data = m_spirvBinaries[stage];
                    out.write((char*)data.data(), data.size() * sizeof(uint32_t));
                    out.flush();
                }
            }

            reflect(stage, m_spirvBinaries[stage]);
        }
#endif
    }
}

void OpenGLShader::creatProgram() {
    GLuint program = glCreateProgram();
    std::vector<GLuint> shaderIDs;

    if (X::GLRendererInfo::Get().ARB_gl_spirv) {
#ifdef glSpecializeShaderARB
        // 支持spriv 用字节码创建shader程序
        for (auto&& [stage, spirv] : m_spirvBinaries) {
            GLuint shaderID = shaderIDs.emplace_back(glCreateShader(stage));
            glShaderBinary(1, &shaderID, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, spirv.data(),
                           spirv.size() * sizeof(uint32_t));
            glSpecializeShaderARB(shaderID, "main", 0, nullptr, nullptr);
            glAttachShader(program, shaderID);
        }
#endif
    } else {
        // 不支持spirv就用GLSL源码创建shader程序
        for (auto&& [stage, source] : m_glslSources) {
            GLuint shaderID = shaderIDs.emplace_back(glCreateShader(stage));
            const char* src = source.c_str();
            glShaderSource(shaderID, 1, &src, nullptr);
            glCompileShader(shaderID);

            GLint isCompiled = 0;
            glGetShaderiv(shaderID, GL_COMPILE_STATUS, &isCompiled);
            if (isCompiled == GL_FALSE) {
                GLint maxLength = 0;
                glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);
                std::vector<GLchar> infoLog(maxLength);
                glGetShaderInfoLog(shaderID, maxLength, &maxLength, infoLog.data());
                X_CORE_ERROR("Shader compilation failed ({}):\n{}", Util::GLShaderStageToString(stage), infoLog.data());
                glDeleteShader(shaderID);
                X_CORE_ASSERT(false);
            }
            glAttachShader(program, shaderID);
        }
    }
    glLinkProgram(program);
    GLint isLinked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE) {
        GLint maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());
        X_CORE_ERROR("Shader linking failed ({}):\n{}", m_filePath, infoLog.data());
        glDeleteProgram(program);
        for (auto id : shaderIDs) {
            glDeleteShader(id);
        }
    }
    for (auto id : shaderIDs) {
        glDetachShader(program, id);
        glDeleteShader(id);
    }
    m_rendererId = program;
}

void OpenGLShader::reflect(GLenum stage, const std::vector<uint32_t>& shaderData) {
    spirv_cross::Compiler compiler(shaderData);
    spirv_cross::ShaderResources resources = compiler.get_shader_resources();
    X_CORE_TRACE("OpenGLShader::reflect - {} {}", Util::GLShaderStageToString(stage), m_filePath);
    X_CORE_TRACE("  {} uniform buffers", resources.uniform_buffers.size());
    X_CORE_TRACE("  {} resources", resources.sampled_images.size());
    X_CORE_TRACE("Uniform buffers:");
    for (const auto& resource : resources.uniform_buffers) {
        const auto& bufferType = compiler.get_type(resource.base_type_id);
        uint32_t bufferSize = compiler.get_declared_struct_size(bufferType);
        uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        int memberCount = bufferType.member_types.size();
        X_CORE_TRACE("  {}", resource.name);
        X_CORE_TRACE("    Size = {}", bufferSize);
        X_CORE_TRACE("    Binding = {}", binding);
        X_CORE_TRACE("    Members = {}", memberCount);
    }
}
