//
// Created by dingrui on 2/23/26.
//

#include "pch.h"
#include <x_engine.h>

#include <glm/gtx/transform.hpp>

#include "platform/opengl/open_gl_shader.h"

class ExampleLayer : public Layer {
public:
    ExampleLayer() : Layer("X-EXAMPLE"), m_camera(-1.6f, 1.6f, -0.9f, 0.9f) {
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
	        -0.5f, -0.5f, 0.0f,
	         0.5f, -0.5f, 0.0f,
	         0.5f,  0.5f, 0.0f,
	        -0.5f,  0.5f, 0.0f
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
		    uniform mat4 u_ViewProjection;
		    uniform mat4 u_Transform;
			out vec3 v_Position;
			out vec4 v_Color;
			void main()
			{
				v_Position = a_Position;
			    v_Color = a_Color;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}
		);

	    const char* fragmentSrc1 = X_GLSL(
			in vec3 v_Position;
			in vec4 v_Color;
			layout(location = 0) out vec4 color;
			void main()
			{
				color = vec4(v_Position * 0.5 + 0.5, 1.0);
			    color = v_Color;
			}
		);
        // clang-format on
        m_shader1.reset(Shader::Create(vertexSrc1, fragmentSrc1));

	    // clang-format off
	    std::string vertexSrc2 = X_GLSL(
			layout(location = 0) in vec3 a_Position;
		    uniform mat4 u_ViewProjection;
		    uniform mat4 u_Transform;
			out vec3 v_Position;
			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}
		);
	    std::string fragmentSrc2 = X_GLSL(
			in vec3 v_Position;
			layout(location = 0) out vec4 color;
			void main()
			{
				color = vec4(0.2, 0.3, 0.8, 1.0);
			}
		);
        // clang-format on
        m_shader2.reset(Shader::Create(vertexSrc2, fragmentSrc2));
    }

    void OnUpdate(Timestep ts) override {
        RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1.0f});
        RenderCommand::Clear();

        m_camera.set_position({0.5f, 0.5f, 0.0f});
        m_camera.set_rotation(45.0f);

        Renderer::BeginScene(m_camera);

        // 第2个shader
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

    	std::dynamic_pointer_cast<OpenGLShader>(m_shader2)->Bind();
    	std::dynamic_pointer_cast<OpenGLShader>(m_shader2)->UploadUniformFloat3("u_Color", m_color);
        for (int y = 0; y < 20; ++y) {
            for (int x = 0; x < 20; ++x) {
                glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
                glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
                // 第2个shader
                Renderer::Submit(m_shader2, m_VAO2, transform);
            }
        }

        // 第1个shader
        Renderer::Submit(m_shader1, m_VAO1);

        Renderer::EndScene();
    }

    void OnImguiRender() override {
    }

    void OnEvent(Event &e) override {
    }

private:
    std::shared_ptr<Shader> m_shader1; // shader1
    std::shared_ptr<VertexArray> m_VAO1; // shader1的VAO

    std::shared_ptr<Shader> m_shader2; // shader2
    std::shared_ptr<VertexArray> m_VAO2; // shader2的VAO

    OrthographicCamera m_camera;

    glm::vec3 m_cameraPosition;
    float m_cameraMoveSpeed{5.0f};
    float m_cameraRotation{0.0f};
    float m_cameraRotationSpeed{180.0f};

	glm::vec3 m_color{0.2f, 0.3f, 0.8f};
};

class Sandbox : public XApplication {
public:
    Sandbox() {
        PushLayer(new ExampleLayer());
    }

    ~Sandbox() override {
    }
};

XApplication *CreateApplication() {
    return new Sandbox();
}
