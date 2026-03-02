//
// Created by rui ding on 2026/2/27.
//

#include "platform/opengl/open_gl_vertex_array.h"

#include "x/renderer/buffer.h"

#include <glad/glad.h>

static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
{
    switch (type)
    {
        case ShaderDataType::kNone:
        {
            X_CORE_ASSERT(false, "Not support ShaderDataType::kNone");
        }
        case ShaderDataType::kFloat:
        {
            return GL_FLOAT;
        }
        case ShaderDataType::kFloat2:
        {
            return GL_FLOAT;
        }
        case ShaderDataType::kFloat3:
        {
            return GL_FLOAT;
        }
        case ShaderDataType::kFloat4:
        {
            return GL_FLOAT;
        }
        case ShaderDataType::kMat3:
        {
            return GL_FLOAT;
        }
        case ShaderDataType::kMat4:
        {
            return GL_FLOAT;
        }
        case ShaderDataType::kInt:
        {
            return GL_INT;
        }
        case ShaderDataType::kInt2:
        {
            return GL_INT;
        }
        case ShaderDataType::kInt3:
        {
            return GL_INT;
        }
        case ShaderDataType::kInt4:
        {
            return GL_INT;
        }
        case ShaderDataType::kBool:
        {
            return GL_BOOL;
        }
    }

    X_CORE_ASSERT(false, "Unknown ShaderDataType!");
    return 0;
}

OpenGLVertexArray::OpenGLVertexArray()
{
    X_PROFILE_FUNCTION();
    glGenVertexArrays(1, &m_rendererID);
}

OpenGLVertexArray::~OpenGLVertexArray()
{
    X_PROFILE_FUNCTION();
    glDeleteVertexArrays(1, &m_rendererID);
}

void OpenGLVertexArray::Bind() const
{
    X_PROFILE_FUNCTION();
    glBindVertexArray(m_rendererID);
}

void OpenGLVertexArray::Unbind() const
{
    X_PROFILE_FUNCTION();
    glBindVertexArray(0);
}

void OpenGLVertexArray::AddVertexBuffer(const X::Ref<VertexBuffer> &vertexBuffer)
{
    X_PROFILE_FUNCTION();
    X_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

    glBindVertexArray(m_rendererID);
    vertexBuffer->Bind();

    const auto &layout = vertexBuffer->GetLayout();
    for (const auto &element : layout)
    {
        switch (element.type)
        {
            case ShaderDataType::kFloat:
            case ShaderDataType::kFloat2:
            case ShaderDataType::kFloat3:
            case ShaderDataType::kFloat4:
            case ShaderDataType::kInt:
            case ShaderDataType::kInt2:
            case ShaderDataType::kInt3:
            case ShaderDataType::kInt4:
            case ShaderDataType::kBool:
            {
                glEnableVertexAttribArray(m_vertexBufferIndex);
                glVertexAttribPointer(m_vertexBufferIndex, element.GetComponentCount(),
                                      ShaderDataTypeToOpenGLBaseType(element.type),
                                      element.normalized ? GL_TRUE : GL_FALSE, layout.GetStride(),
                                      reinterpret_cast<const void *>(element.offset));
                m_vertexBufferIndex++;
                break;
            }
            case ShaderDataType::kMat3:
            case ShaderDataType::kMat4:
            {
                uint8_t count = element.GetComponentCount();
                for (uint8_t i = 0; i < count; i++)
                {
                    glEnableVertexAttribArray(m_vertexBufferIndex);
                    glVertexAttribPointer(m_vertexBufferIndex, count, ShaderDataTypeToOpenGLBaseType(element.type),
                                          element.normalized ? GL_TRUE : GL_FALSE, layout.GetStride(),
                                          reinterpret_cast<const void *>(sizeof(float) * count * i));
                    glVertexAttribDivisor(m_vertexBufferIndex, 1);
                    m_vertexBufferIndex++;
                }
                break;
            }
            default:
                X_CORE_ASSERT(false, "Unknown ShaderDataType!");
        }
    }
    m_vertexBuffers.push_back(vertexBuffer);
}

void OpenGLVertexArray::SetIndexBuffer(const X::Ref<IndexBuffer> &indexBuffer)
{
    X_PROFILE_FUNCTION();
    glBindVertexArray(m_rendererID);
    indexBuffer->Bind();
    m_indexBuffer = indexBuffer;
}