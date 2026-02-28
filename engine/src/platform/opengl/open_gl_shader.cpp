//
// Created by dingrui on 2/28/26.
//

#include "platform/opengl/open_gl_shader.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.inl>

#include "x/x_log.h"
#include "x/core.h"

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
    X_CORE_ASSERT(false, "Unknown shader type!");
    return 0;
}

OpenGLShader::OpenGLShader(const std::string &filepath)
{
    std::string source        = readFile(filepath);
    auto        shaderSources = preProcess(source);
    compile(shaderSources);
}

OpenGLShader::OpenGLShader(const std::string &vertexSrc, const std::string fragmentSrc)
{
    std::unordered_map<int, std::string> shaderSources;
    shaderSources[GL_VERTEX_SHADER]   = vertexSrc;
    shaderSources[GL_FRAGMENT_SHADER] = fragmentSrc;
    compile(shaderSources);
}

OpenGLShader::~OpenGLShader()
{
    if (m_rendererId > 0)
    {
        glDeleteProgram(m_rendererId);
    }
}

void OpenGLShader::Bind() const
{
    glUseProgram(m_rendererId);
}

void OpenGLShader::Unbind() const
{
    glUseProgram(0);
}

void OpenGLShader::UploadUniformInt(const std::string &name, int value)
{
    GLint location = glGetUniformLocation(m_rendererId, name.c_str());
    glUniform1i(location, value);
}

void OpenGLShader::UploadUniformFloat(const std::string &name, float value)
{
    GLint location = glGetUniformLocation(m_rendererId, name.c_str());
    glUniform1f(location, value);
}

void OpenGLShader::UploadUniformFloat2(const std::string &name, const glm::vec2 &value)
{
    GLint location = glGetUniformLocation(m_rendererId, name.c_str());
    glUniform2f(location, value.x, value.y);
}

void OpenGLShader::UploadUniformFloat3(const std::string &name, const glm::vec3 &value)
{
    GLint location = glGetUniformLocation(m_rendererId, name.c_str());
    glUniform3f(location, value.x, value.y, value.z);
}

void OpenGLShader::UploadUniformFloat4(const std::string &name, const glm::vec4 &value)
{
    GLint location = glGetUniformLocation(m_rendererId, name.c_str());
    glUniform4f(location, value.x, value.y, value.z, value.w);
}

void OpenGLShader::UploadUniformMat3(const std::string &name, const glm::mat3 &matrix)
{
    GLint location = glGetUniformLocation(m_rendererId, name.c_str());
    glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

void OpenGLShader::UploadUniformMat4(const std::string &name, const glm::mat4 &matrix)
{
    GLint location = glGetUniformLocation(m_rendererId, name.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

std::string OpenGLShader::readFile(const std::string &filepath)
{
    std::string   ret;
    std::ifstream in(filepath, std::ios::in | std::ios::binary);
    X_CORE_ASSERT(in, "Could not open file {}", filepath);
    in.seekg(0, std::ios::end);
    ret.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&ret[0], ret.size());
    in.close();
    return ret;
}

std::unordered_map<int, std::string> OpenGLShader::preProcess(const std::string &glslSrc)
{
    std::unordered_map<int, std::string> shaderSources;
    const char                          *typeToken       = "#type";  // vertex跟frag的区分用#type vertex跟#type frag
    size_t                               typeTokenLength = strlen(typeToken);
    size_t                               pos             = glslSrc.find(typeToken, 0);
    while (pos != std::string::npos)
    {
        size_t eol = glslSrc.find_first_of("\r\n", pos);
        X_CORE_ASSERT(eol != std::string::npos, "Syntax error");
        size_t      begin = pos + typeTokenLength + 1;
        std::string type  = glslSrc.substr(begin, eol - begin);  // vertex or frag
        X_CORE_ASSERT(shaderTypeFromString(type), "Invalid shader type specified");
        size_t nextLinePos = glslSrc.find_first_not_of("\r\n", eol);
        pos                = glslSrc.find(typeToken, nextLinePos);
        shaderSources[shaderTypeFromString(type)] =
            glslSrc.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? glslSrc.size() - 1 : nextLinePos));
    }
    return shaderSources;
}

void OpenGLShader::compile(const std::unordered_map<int, std::string> &shaderSources)
{
    // Vertex and fragment shaders are successfully compiled.
    // Now time to link them together into a program.
    // Get a program object.
    GLuint           program = glCreateProgram();
    std::vector<int> glShaderIds(shaderSources.size());
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
        glShaderIds.push_back(shader);
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
    }
}
