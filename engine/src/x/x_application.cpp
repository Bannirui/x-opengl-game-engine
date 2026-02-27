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
        // pos(xyz)
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f,
    };
    // clang-format on

    // VBO
    m_vertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

    uint32_t indices[3] = {0, 1, 2};
    // EBO
    m_indexBuffer.reset(IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));

    // clang-format off
    const char* vertexSrc = X_GLSL(
	    layout(location = 0) in vec3 a_Position;
		out vec3 v_Position;
		void main()
		{
			v_Position = a_Position;
			gl_Position = vec4(a_Position, 1.0);
		}
	);

    const char* fragmentSrc = X_GLSL(
		layout(location = 0) out vec4 color;
		in vec3 v_Position;
		void main()
		{
			color = vec4(v_Position * 0.5 + 0.5, 1.0);
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
