//
// Created by dingrui on 2/28/26.
//

#include "sandbox_2D.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.inl>

#include "platform/opengl/open_gl_shader.h"

Sandbox2D::Sandbox2D()
    : Layer("Sandbox2D"), m_cameraController(1280.0f / 720.0f) {
}

Sandbox2D::~Sandbox2D() {
}

void Sandbox2D::OnAttach() {
    m_squareVA = VertexArray::Create();

    // clang-format off
	// float squareVertices[] = {
	//      // pos(xyz)          // uv(xy)
	//     -0.5f, -0.5f,  0.0f,  0.0f,  0.0f,
	//      0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	//      0.5f,  0.5f,  0.0f,  1.0f,  1.0f,
	//     -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	// };
	float squareVertices[] = {
	     // pos(xyz)          // uv(xy)
	    -0.5f, -0.5f,  0.0f,
	     0.5f, -0.5f,  0.0f,
	     0.5f,  0.5f,  0.0f,
	    -0.5f,  0.5f,  0.0f,
	};
	uint32_t squareIndices[] = {0, 1, 2, 2, 3, 0};
    // clang-format on

    X::Ref<VertexBuffer> squareVB;
    squareVB.reset(VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
    squareVB->SetLayout({{ShaderDataType::kFloat3, "a_Position"}});
    m_squareVA->AddVertexBuffer(squareVB);

    X::Ref<IndexBuffer> squareIB;
    squareIB.reset(IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
    m_squareVA->SetIndexBuffer(squareIB);

    m_flatShader = Shader::Create("asset/shader/FlatColor.glsl");
}

void Sandbox2D::OnDetach() {
}

void Sandbox2D::OnUpdate(Timestep ts) {
    // update
    m_cameraController.OnUpdate(ts);
    // render
    RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1.0f});
    RenderCommand::Clear();

    Renderer::BeginScene(m_cameraController.get_camera());


    std::dynamic_pointer_cast<OpenGLShader>(m_flatShader)->Bind();
    std::dynamic_pointer_cast<OpenGLShader>(m_flatShader)->UploadUniformFloat3("u_Color", m_squareColor);

    Renderer::EndScene();
}

void Sandbox2D::OnImguiRender() {
    ImGui::Begin("Settings");
    ImGui::ColorEdit4("Square color", glm::value_ptr(m_squareColor));
    ImGui::End();
}

void Sandbox2D::OnEvent(Event &e) {
    m_cameraController.OnEvent(e);
}
