//
// Created by dingrui on 2/23/26.
//

#include "x/x_application.h"

#include "x/renderer/render_command.h"
#include "x/renderer/renderer.h"
#include "renderer/vertex_array.h"
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
	:m_camera(-1.6f, 1.6f, -0.9f, 0.9f)
{
    X_CORE_ASSERT(!s_instance, "Application already exists");
    s_instance = this;
    m_window   = std::unique_ptr<Window>(Window::Create());
	m_window->SetEventCallback([this](Event& e){ this->OnEvent(e); });
    // todo 创建input
    Input::Create();

    m_ImGuiLayer = new ImGuiLayer();
    PushOverlay(m_ImGuiLayer);

    // 三角形
    // clang-format off
    float vertices1[] = {
         // pos(xyz)        // color(rgba)
        -0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
         0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
         0.0f,  0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f,
    };
    uint32_t indices1[] = {0, 1, 2};
    // clang-format on
    // VAO
    m_VAO1.reset(VertexArray::Create());
    // VAO托管VBO
    std::shared_ptr<VertexBuffer> vertexBuffer;
    vertexBuffer.reset(VertexBuffer::Create(vertices1, sizeof(vertices1)));
    BufferLayout layout = {{ShaderDataType::kFloat3, "a_Position"}, {ShaderDataType::kFloat4, "a_Color"}};
    vertexBuffer->SetLayout(layout);
    m_VAO1->AddVertexBuffer(vertexBuffer);
    // VAO托管EBO
    std::shared_ptr<IndexBuffer> indexBuffer;
    indexBuffer.reset(IndexBuffer::Create(indices1, sizeof(indices1) / sizeof(uint32_t)));
    m_VAO1->SetIndexBuffer(indexBuffer);

    // 正方形
    // clang-format off
    float vertices2[] = {
        -0.75f, -0.75f, 0.0f,
         0.75f, -0.75f, 0.0f,
         0.75f,  0.75f, 0.0f,
        -0.75f,  0.75f, 0.0f
    };
    uint32_t indices2[] = {0, 1, 2, 2, 3, 0};
    // clang-format on
    // VAO
    m_VAO2.reset(VertexArray::Create());
    // VAO托管VBO
    std::shared_ptr<VertexBuffer> squareVB;
    squareVB.reset(VertexBuffer::Create(vertices2, sizeof(vertices2)));
    squareVB->SetLayout({{ShaderDataType::kFloat3, "a_Position"}});
    m_VAO2->AddVertexBuffer(squareVB);
    // VAO托管EBO
    std::shared_ptr<IndexBuffer> squareIB;
    squareIB.reset(IndexBuffer::Create(indices2, sizeof(indices2) / sizeof(uint32_t)));
    m_VAO2->SetIndexBuffer(squareIB);

    // clang-format off
    const char* vertexSrc1 = X_GLSL(
	    layout(location = 0) in vec3 a_Position;
	    layout(location = 1) in vec4 a_Color;
	    uniform mat4 u_viewProjection;
		out vec3 v_Position;
		out vec4 v_Color;
		void main()
		{
			v_Position = a_Position;
		    v_Color = a_Color;
			gl_Position = u_viewProjection * vec4(a_Position, 1.0);
		}
	);

    const char* fragmentSrc1 = X_GLSL(
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
    m_shader1.reset(new Shader(vertexSrc1, fragmentSrc1));

    // clang-format off
    std::string vertexSrc2 = X_GLSL(
		layout(location = 0) in vec3 a_Position;
	    uniform mat4 u_viewProjection;
		out vec3 v_Position;
		void main()
		{
			v_Position = a_Position;
			gl_Position = u_viewProjection * vec4(a_Position, 1.0);
		}
	);
    std::string fragmentSrc2 = X_GLSL(
		layout(location = 0) out vec4 color;
		in vec3 v_Position;
		void main()
		{
			color = vec4(0.2, 0.3, 0.8, 1.0);
		}
	);
    // clang-format on
    m_shader2.reset(new Shader(vertexSrc2, fragmentSrc2));
}

XApplication::~XApplication() {}

void XApplication::Run()
{
    while (m_running)
    {
        RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1.0f});
        RenderCommand::Clear();

    	m_camera.set_position({0.5f, 0.5f, 0.0f});
    	m_camera.set_rotation(45.0f);

        Renderer::BeginScene(m_camera);

        // 第2个shader
        Renderer::Submit(m_shader2, m_VAO2);
        // 第1个shader
        Renderer::Submit(m_shader1, m_VAO1);

        Renderer::EndScene();

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
	dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent &e){ return this->onWindowClose(e); });
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
