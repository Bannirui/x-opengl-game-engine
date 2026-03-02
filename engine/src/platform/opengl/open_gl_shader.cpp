//
// Created by dingrui on 2/28/26.
//

#include "platform/opengl/open_gl_shader.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.inl>

#include "x/core/x_log.h"
#include "x/core/core.h"

// shader源码用#type vertex跟#type fragment区分
static int shaderTypeFromString(const std::string &type)
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

OpenGLShader::OpenGLShader(const std::string &filepath)
{
    X_PROFILE_FUNCTION();
    std::string source        = readFile(filepath);
    auto        shaderSources = preProcess(source);
    compile(shaderSources);
    // 文件名
    auto lastSlash = filepath.find_last_of("/\\");
    lastSlash      = lastSlash == std::string::npos ? 0 : lastSlash + 1;
    auto lastDot   = filepath.rfind('.');
    auto count     = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
    m_name         = filepath.substr(lastSlash, count);
}

OpenGLShader::OpenGLShader(const std::string &name, const std::string &vertexSrc, const std::string fragmentSrc)
    : m_name(name)
{
    X_PROFILE_FUNCTION();
    std::unordered_map<int, std::string> shaderSources;
    shaderSources[GL_VERTEX_SHADER]   = vertexSrc;
    shaderSources[GL_FRAGMENT_SHADER] = fragmentSrc;
    compile(shaderSources);
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

void OpenGLShader::SetInt(const std::string &name, int value)
{
    X_PROFILE_FUNCTION();
    uploadUniformInt(name, value);
}

void OpenGLShader::SetFloat(const std::string &name, float value)
{
    X_PROFILE_FUNCTION();
    uploadUniformFloat(name, value);
}

void OpenGLShader::SetFloat3(const std::string &name, const glm::vec3 &value)
{
    X_PROFILE_FUNCTION();
    uploadUniformFloat3(name, value);
}

void OpenGLShader::SetFloat4(const std::string &name, const glm::vec4 &value)
{
    X_PROFILE_FUNCTION();
    uploadUniformFloat4(name, value);
}

void OpenGLShader::SetMat4(const std::string &name, const glm::mat4 &value)
{
    X_PROFILE_FUNCTION();
    uploadUniformMat4(name, value);
}

void OpenGLShader::uploadUniformInt(const std::string &name, int value)
{
    GLint location = glGetUniformLocation(m_rendererId, name.c_str());
    glUniform1i(location, value);
}

void OpenGLShader::uploadUniformFloat(const std::string &name, float value)
{
    GLint location = glGetUniformLocation(m_rendererId, name.c_str());
    glUniform1f(location, value);
}

void OpenGLShader::uploadUniformFloat2(const std::string &name, const glm::vec2 &value)
{
    GLint location = glGetUniformLocation(m_rendererId, name.c_str());
    glUniform2f(location, value.x, value.y);
}

void OpenGLShader::uploadUniformFloat3(const std::string &name, const glm::vec3 &value)
{
    GLint location = glGetUniformLocation(m_rendererId, name.c_str());
    glUniform3f(location, value.x, value.y, value.z);
}

void OpenGLShader::uploadUniformFloat4(const std::string &name, const glm::vec4 &value)
{
    GLint location = glGetUniformLocation(m_rendererId, name.c_str());
    glUniform4f(location, value.x, value.y, value.z, value.w);
}

void OpenGLShader::uploadUniformMat3(const std::string &name, const glm::mat3 &matrix)
{
    GLint location = glGetUniformLocation(m_rendererId, name.c_str());
    glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

void OpenGLShader::uploadUniformMat4(const std::string &name, const glm::mat4 &matrix)
{
    GLint location = glGetUniformLocation(m_rendererId, name.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

std::string OpenGLShader::readFile(const std::string &filepath)
{
    X_PROFILE_FUNCTION();
    std::string   ret;
    std::ifstream in(filepath, std::ios::in | std::ios::binary);
    X_CORE_ASSERT(in, "Could not open file {}", filepath);
    in.seekg(0, std::ios::end);
    size_t size = in.tellg();
    X_CORE_ASSERT(size != 1, "Could not read from file {}", filepath);
    ret.resize(size);
    in.seekg(0, std::ios::beg);
    in.read(&ret[0], ret.size());
    in.close();
    return ret;
}

std::unordered_map<int, std::string> OpenGLShader::preProcess(const std::string &glslSrc)
{
    X_PROFILE_FUNCTION();
    std::unordered_map<int, std::string> shaderSources;
    const char                          *typeToken       = "#type";  // vertex跟frag的区分用#type vertex跟#type frag
    size_t                               typeTokenLength = strlen(typeToken);
    size_t                               pos             = glslSrc.find(typeToken, 0);  // 找到第一个#type标识
    while (pos != std::string::npos)
    {
        size_t eol = glslSrc.find_first_of("\r\n", pos);  // 定位位#type那一行最后 中间就是#type的内容
        X_CORE_ASSERT(eol != std::string::npos, "Syntax error");
        size_t      begin = pos + typeTokenLength + 1;
        std::string type  = glslSrc.substr(begin, eol - begin);  // vertex or frag
        X_CORE_ASSERT(shaderTypeFromString(type), "Invalid shader type specified");
        size_t nextLinePos = glslSrc.find_first_not_of("\r\n", eol);
        X_CORE_ASSERT(nextLinePos != std::string::npos, "Syntax error");
        pos = glslSrc.find(typeToken, nextLinePos);
        shaderSources[shaderTypeFromString(type)] =
            (pos == std::string::npos) ? glslSrc.substr(nextLinePos) : glslSrc.substr(nextLinePos, pos - nextLinePos);
    }
    return shaderSources;
}

void OpenGLShader::compile(const std::unordered_map<int, std::string> &shaderSources)
{
    X_PROFILE_FUNCTION();
    X_CORE_ASSERT(shaderSources.size() <= 2, "We only support 2 shaders for now");
    // Vertex and fragment shaders are successfully compiled.
    // Now time to link them together into a program.
    // Get a program object.
    GLuint                program = glCreateProgram();
    std::array<GLenum, 2> glShaderIds;
    int                   glShaderIDIndex = 0;
    for (auto &kv : shaderSources)
    {
        int               type   = kv.first;
        const std::string source = kv.second;
        // Create an empty vertex shader handle
        GLuint shader = glCreateShader(type);
        // Send the vertex shader source code to GL
        // Note that std::string's .c_str is NULL character terminated.
        const GLchar *sourceCStr = source.c_str();
        glShaderSource(shader, 1, &sourceCStr, 0);
        // Compile the vertex shader
        glCompileShader(shader);
        GLint isCompiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
        if (isCompiled == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
            // The maxLength includes the NULL character
            std::vector<GLchar> infoLog(maxLength);
            glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
            // We don't need the shader anymore.
            glDeleteShader(shader);

            X_CORE_ERROR("{0}", infoLog.data());
            X_CORE_ASSERT(false, "Vertex shader compilation failure!");
            break;
        }
        // Attach our shaders to our program
        glAttachShader(program, shader);
        glShaderIds[glShaderIDIndex++] = shader;
    }
    m_rendererId = program;

    // Link our program
    glLinkProgram(program);

    // Note the different functions here: glGetProgram* instead of glGetShader*.
    GLint isLinked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, (int *)&isLinked);
    if (isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

        // We don't need the program anymore.
        glDeleteProgram(program);
        // Don't leak shaders either.
        for (auto id : glShaderIds)
        {
            glDeleteShader(id);
        }

        X_CORE_ERROR("{0}", infoLog.data());
        X_CORE_ASSERT(false, "Shader link failure!");
        return;
    }
    // Always detach shaders after a successful link.
    for (auto id : glShaderIds)
    {
        glDetachShader(program, id);
        glDeleteShader(id);
    }
}
