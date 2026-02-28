//
// Created by dingrui on 2/28/26.
//

#include "x/renderer/renderer_2D.h"

#include "buffer.h"
#include "render_command.h"
#include "platform/opengl/open_gl_shader.h"
#include "x/renderer/shader.h"
#include "x/renderer/vertex_array.h"
#include "x/core.h"

struct Renderer2DStorage {
    X::Ref<VertexArray> quadVertexArray;
    X::Ref<Shader> flatColorShader;
};

static Renderer2DStorage *s_data;

void Renderer2D::Init() {
    // clang-format off
	float squareVertices[] = {
	     // pos(xyz)          // uv(xy)
	    -0.5f, -0.5f,  0.0f,
	     0.5f, -0.5f,  0.0f,
	     0.5f,  0.5f,  0.0f,
	    -0.5f,  0.5f,  0.0f,
	};
	uint32_t squareIndices[] = {0, 1, 2, 2, 3, 0};
    // clang-format on

    s_data = new Renderer2DStorage();
    s_data->quadVertexArray = VertexArray::Create();

    X::Ref<VertexBuffer> squareVB;
    squareVB.reset(VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
    squareVB->SetLayout({{ShaderDataType::kFloat3, "a_Position"}});
    s_data->quadVertexArray->AddVertexBuffer(squareVB);

    X::Ref<IndexBuffer> squareIB;
    squareIB.reset(IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
    s_data->quadVertexArray->SetIndexBuffer(squareIB);

    s_data->flatColorShader = Shader::Create("asset/shader/FlatColor.glsl");
}

void Renderer2D::Shutdown() {
    delete s_data;
}

void Renderer2D::BeginScene(const OrthographicCamera &camera) {
    std::dynamic_pointer_cast<OpenGLShader>(s_data->flatColorShader)->Bind();
    std::dynamic_pointer_cast<OpenGLShader>(s_data->flatColorShader)->UploadUniformMat4(
        "u_ViewProjection", camera.get_viewProjectionMatrix());
    std::dynamic_pointer_cast<OpenGLShader>(s_data->flatColorShader)->UploadUniformMat4(
        "u_Transform", glm::mat4(1.0f));
}

void Renderer2D::EndScene() {
}

void Renderer2D::DrawQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color) {
    DrawQuad({position.x, position.y, 0.0f}, size, color);
}

void Renderer2D::DrawQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color) {
    std::dynamic_pointer_cast<OpenGLShader>(s_data->flatColorShader)->Bind();
    std::dynamic_pointer_cast<OpenGLShader>(s_data->flatColorShader)->UploadUniformFloat4("u_Color", color);

    s_data->quadVertexArray->Bind();
    RenderCommand::DrawIndexed(s_data->quadVertexArray);
}
