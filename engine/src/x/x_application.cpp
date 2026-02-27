//
// Created by dingrui on 2/23/26.
//

#include "x/x_application.h"

#include "x/core.h"
#include "x/layer.h"
#include "x/x_log.h"
#include "x/events/application_event.h"
#include "x/window.h"
#include "x/input.h"
#include "x/imgui/im_gui_layer.h"
#include "x/renderer/shader.h"
#include "x/renderer/buffer.h"

XApplication *XApplication::s_instance = nullptr;

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

XApplication::XApplication()
{
    X_CORE_ASSERT(!s_instance, "Application already exists");
    s_instance = this;
    m_window   = std::unique_ptr<Window>(Window::Create());
    m_window->SetEventCallback(X_BIND_EVENT_FN(XApplication::OnEvent));
    // todo 创建input
    Input::Create();

    m_ImGuiLayer = new ImGuiLayer();
    PushOverlay(m_ImGuiLayer);

    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);
    X_CORE_ASSERT(m_VAO, "创建VAO失败");

    // clang-format off
    float vertices[] = {
         // pos(xyz)        // color(rgba)
        -0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
         0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
         0.0f,  0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f,
    };
    // clang-format on

    // VBO
    m_vertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));
    {
        BufferLayout layout = {{ShaderDataType::kFloat3, "a_Position"}, {ShaderDataType::kFloat4, "a_Color"}};
        m_vertexBuffer->SetLayout(layout);
    }

    uint32_t    index  = 0;
    const auto &layout = m_vertexBuffer->GetLayout();
    for (const auto &element : layout)
    {
        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, element.GetComponentCount(), ShaderDataTypeToOpenGLBaseType(element.type),
                              element.normalized ? GL_TRUE : GL_FALSE, layout.GetStride(),
                              reinterpret_cast<const void *>(element.offset));
        ++index;
    }

    uint32_t indices[3] = {0, 1, 2};
    // EBO
    m_indexBuffer.reset(IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));

    // clang-format off
    const char* vertexSrc = X_GLSL(
	    layout(location = 0) in vec3 a_Position;
	    layout(location = 1) in vec4 a_Color;
		out vec3 v_Position;
		out vec4 v_Color;
		void main()
		{
			v_Position = a_Position;
		    v_Color = a_Color;
			gl_Position = vec4(a_Position, 1.0);
		}
	);

    const char* fragmentSrc = X_GLSL(
		layout(location = 0) out vec4 color;
		in vec3 v_Position;
		in vec4 v_Color;
		void main()
		{
			color = vec4(v_Position * 0.5 + 0.5, 1.0);
		    color = v_Color;
		}
	);
    // clang-format on

    m_shader.reset(new Shader(vertexSrc, fragmentSrc));
}

XApplication::~XApplication() {}

void XApplication::Run()
{
    while (m_running)
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, m_indexBuffer->GetCount(), GL_UNSIGNED_INT, nullptr);

        for (Layer *layer : m_layerStack)
        {
            layer->OnUpdate();
        }

        m_ImGuiLayer->Begin();
        for (Layer *layer : m_layerStack)
        {
            layer->OnImguiRender();
        }
        m_ImGuiLayer->End();

        m_window->OnUpdate();
    }
}

void XApplication::OnEvent(Event &e)
{
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowCloseEvent>(X_BIND_EVENT_FN(XApplication::onWindowClose));
    for (auto it = m_layerStack.end(); it != m_layerStack.begin();)
    {
        (*--it)->OnEvent(e);
        if (e.Handled)
        {
            break;
        }
    }
}

void XApplication::PushLayer(Layer *layer)
{
    m_layerStack.PushLayer(layer);
}

void XApplication::PushOverlay(Layer *layer)
{
    m_layerStack.PushOverlay(layer);
}

bool XApplication::onWindowClose(WindowCloseEvent &e)
{
    m_running = false;
    return true;
}
