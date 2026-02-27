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
    glGenVertexArrays(1, &m_rendererID);
}

OpenGLVertexArray::~OpenGLVertexArray()
{
    glDeleteVertexArrays(1, &m_rendererID);
}

void OpenGLVertexArray::Bind() const
{
    glBindVertexArray(m_rendererID);
}

void OpenGLVertexArray::Unbind() const
{
    glBindVertexArray(0);
}

void OpenGLVertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer> &vertexBuffer)
{
    X_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

    glBindVertexArray(m_rendererID);
    vertexBuffer->Bind();

    uint32_t    index  = 0;
    const auto &layout = vertexBuffer->GetLayout();
    for (const auto &element : layout)
    {
        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, element.GetComponentCount(), ShaderDataTypeToOpenGLBaseType(element.type),
                              element.normalized ? GL_TRUE : GL_FALSE, layout.GetStride(),
                              reinterpret_cast<const void *>(element.offset));
        index++;
    }
    m_vertexBuffers.push_back(vertexBuffer);
}

void OpenGLVertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer> &indexBuffer)
{
    glBindVertexArray(m_rendererID);
    indexBuffer->Bind();
    m_indexBuffer = indexBuffer;
}