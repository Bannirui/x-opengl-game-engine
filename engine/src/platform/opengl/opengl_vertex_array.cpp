//
// Created by rui ding on 2026/2/27.
//

#include "platform/opengl/opengl_vertex_array.h"

#include "x/core/assert.h"
#include "x/renderer/buffer/buffer.h"

#include <glad/glad.h>

/**
 * VBO顶点的数据类型 比如
 *   - pos用xyz这3个float 即pos这个分量的描述类型是float
 *   - color用rgba这4个float 即color这个分量的描述类型是float
 *   - ...
 * @param type VBO顶点数据里面的分量数据类型
 * @return VBO顶点的分量用的是什么数据类型
 */
static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type) {
    switch (type) {
        case ShaderDataType::kNone: {
            CORE_ERROR("Not support ShaderDataType::kNone");
        }
        case ShaderDataType::kFloat: {
            return GL_FLOAT;
        }
        case ShaderDataType::kFloat2: {
            return GL_FLOAT;
        }
        case ShaderDataType::kFloat3: {
            return GL_FLOAT;
        }
        case ShaderDataType::kFloat4: {
            return GL_FLOAT;
        }
        case ShaderDataType::kMat3: {
            return GL_FLOAT;
        }
        case ShaderDataType::kMat4: {
            return GL_FLOAT;
        }
        case ShaderDataType::kInt: {
            return GL_INT;
        }
        case ShaderDataType::kInt2: {
            return GL_INT;
        }
        case ShaderDataType::kInt3: {
            return GL_INT;
        }
        case ShaderDataType::kInt4: {
            return GL_INT;
        }
        case ShaderDataType::kBool: {
            return GL_BOOL;
        }
    }
    CORE_ERROR("Unknown ShaderDataType!");
    return 0;
}

OpenGLVertexArray::OpenGLVertexArray() {
    X_PROFILE_FUNCTION();
    // 让OpenGL在显存开辟空间作为vertex array object 用id引用
    glGenVertexArrays(1, &m_rendererID);
}

OpenGLVertexArray::~OpenGLVertexArray() {
    X_PROFILE_FUNCTION();
    glDeleteVertexArrays(1, &m_rendererID);
}

void OpenGLVertexArray::Bind() const {
    X_PROFILE_FUNCTION();
    // 在渲染时只要调用一下 GPU就知道去哪个VBO按照什么格式读取顶点
    glBindVertexArray(m_rendererID);
}

void OpenGLVertexArray::Unbind() const {
    X_PROFILE_FUNCTION();
    glBindVertexArray(0);
}

/**
 * 把VBO告诉VAO 让VAO知道
 *   - 具体的顶点数据
 *   - 这些顶点数据怎么布局的 pos占多少 color占多少 法线占多少
 * VAO属性绑定
 * 负责告诉OpenGL怎么把VBO里面的数据解释成着色器shader的各个属性
 *   - 把VBO绑定到VAO
 *   - 然后遍历VBO的顶点布局信息 对每个属性逐一处理 让OpenGL建立显存中每段字节->着色器attribute location的映射
 * 所以配置而言配置的就是让shader的attribute知道怎么使用VBO
 * @param vertexBuffer VBO VBO里面不仅有多个顶点的数据信息 还有这些每个顶点的有多少个分量
 */
void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) {
    X_PROFILE_FUNCTION();
    // 必须得有布局 VBO中不仅需要顶点数据 还要有这些顶点数据是怎么布局的
    CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");
    // VAO插槽
    glBindVertexArray(m_rendererID);
    // VBO插槽
    vertexBuffer->Bind();

    // OpenGL就是个状态机 绑定VAO插槽和VBO插槽 后面才知道哪个VBO的attribute怎么解释
    const auto& layout = vertexBuffer->GetLayout();
    // for循环保证了分量的遍历顺序 跟shader的GLSL里面的location对应起来 比如依次是a_Position a_Color...
    for (const auto& element : layout) {
        switch (element.type) {
            case ShaderDataType::kFloat:
            case ShaderDataType::kFloat2:
            case ShaderDataType::kFloat3:
            case ShaderDataType::kFloat4: {
                glEnableVertexAttribArray(m_vertexBufferIndex);
                // 告诉OpenGL怎么理解顶点数据
                glVertexAttribPointer(
                    m_vertexBufferIndex,  // 对应着色器的location 比如0,1,2...就对应着色器location 0,1,2...
                    element.GetComponentCount(),  // 顶点的这个分量有多少个数据
                    ShaderDataTypeToOpenGLBaseType(element.type),  // 数据类型
                    element.normalized ? GL_TRUE : GL_FALSE,  // 是否归一化
                    layout.GetStride(), reinterpret_cast<const void*>(element.offset)  // 这个顶点分量在顶点中的偏移
                );
                // 比如参数依次是0 3 GL_FLOAT GL_TRUE 0
                // 就是让OpenGL去显存VBO里面从每个顶点的偏移0开始读连续3个float传给shader的location=0
                m_vertexBufferIndex++;
                break;
            }
            case ShaderDataType::kInt:
            case ShaderDataType::kInt2:
            case ShaderDataType::kInt3:
            case ShaderDataType::kInt4:
            case ShaderDataType::kBool: {
                glEnableVertexAttribArray(m_vertexBufferIndex);
                glVertexAttribIPointer(m_vertexBufferIndex, element.GetComponentCount(),
                                       ShaderDataTypeToOpenGLBaseType(element.type), layout.GetStride(),
                                       reinterpret_cast<const void*>(element.offset));
                m_vertexBufferIndex++;
                break;
            }
            case ShaderDataType::kMat3:
            case ShaderDataType::kMat4: {
                uint8_t count = element.GetComponentCount();
                for (uint8_t i = 0; i < count; i++) {
                    // OpenGL默认是关闭了顶点属性的 所以要告诉OpenGL启用attribute输入
                    // 这样shader程序就可以通过location=x取到显存里面的顶点数据
                    glEnableVertexAttribArray(m_vertexBufferIndex);
                    glVertexAttribPointer(m_vertexBufferIndex, count, ShaderDataTypeToOpenGLBaseType(element.type),
                                          element.normalized ? GL_TRUE : GL_FALSE, layout.GetStride(),
                                          reinterpret_cast<const void*>(element.offset + sizeof(float) * count * i));
                    glVertexAttribDivisor(m_vertexBufferIndex, 1);
                    m_vertexBufferIndex++;
                }
                break;
            }
            default:
                CORE_ERROR("Unknown ShaderDataType!");
        }
    }
    m_vertexBuffers.push_back(vertexBuffer);
}

void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) {
    X_PROFILE_FUNCTION();
    // 激活VAO在OpenGL的插槽
    glBindVertexArray(m_rendererID);
    // 激活EBO在OpenGL的插槽
    indexBuffer->Bind();
    m_indexBuffer = indexBuffer;
}