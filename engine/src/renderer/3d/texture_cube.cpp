//
// Created by rui ding on 2026/6/1.
//

#include "x/renderer/texture_cube.h"

#include "glm/gtc/type_ptr.hpp"

#include <glad/glad.h>

#include <stb_image.h>

static constexpr float CUBEMAP_CAPTURE_PROJECTION[16] = {
    1.0f, 0.0f,  0.0f,  0.0f,  //
    0.0f, -1.0f, 0.0f,  0.0f,  //
    0.0f, 0.0f,  -0.5f, 0.0f,  //
    0.0f, 0.0f,  0.0f,  1.0f   //
};

static const glm::mat4 CAPTURE_VIEWS[6] = {
    glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),   // +X
    glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),  // -X
    glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),    // +Y
    glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),  // -Y
    glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),   // +Z
    glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),  // -Z
};

X::Ref<TextureCube> TextureCube::Create(uint32_t size, bool hdr) {
    X::Ref<TextureCube> cube = X::CreateRef<TextureCube>();
    cube->m_size = size;
    cube->m_hdr = hdr;

    GLenum internalFormat = hdr ? GL_RGB16F : GL_RGB8;
    GLenum format = GL_RGB;

    glGenTextures(1, &cube->m_rendererID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cube->m_rendererID);
    for (int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, size, size, 0, format, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    if (hdr) {
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }

    return cube;
}

X::Ref<TextureCube> TextureCube::CreateFromEquirectangular(const std::string& hdrPath) {
    X_PROFILE_FUNCTION();

    stbi_set_flip_vertically_on_load(true);
    int width, height, channels;
    float* data = stbi_loadf(hdrPath.c_str(), &width, &height, &channels, 0);
    if (!data) {
        X_CORE_ERROR("Failed to load HDR file: {}", hdrPath);
        return nullptr;
    }

    // Create HDR 2D texture from equirectangular data
    GLuint hdrTexture;
    glGenTextures(1, &hdrTexture);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    stbi_image_free(data);

    uint32_t cubeSize = 512;
    X::Ref<TextureCube> cube = TextureCube::Create(cubeSize, true);

    // Create capture FBO
    GLuint captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, cubeSize, cubeSize);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    // Simple equirect-to-cubemap shader (inline)
    const char* equiVert = R"(
#version 410 core
layout(location = 0) in vec3 a_Position;
out vec3 v_WorldPos;
uniform mat4 u_Projection;
uniform mat4 u_View;
void main() {
    v_WorldPos = a_Position;
    vec4 clipPos = u_Projection * u_View * vec4(a_Position, 1.0);
    gl_Position = clipPos.xyww;
}
)";
    const char* equiFrag = R"(
#version 410 core
in vec3 v_WorldPos;
out vec4 o_Color;
uniform sampler2D u_EquirectangularMap;
const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 dir) {
    vec2 uv = vec2(atan(dir.z, dir.x), asin(dir.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}
void main() {
    vec2 uv = SampleSphericalMap(normalize(v_WorldPos));
    vec3 color = texture(u_EquirectangularMap, uv).rgb;
    o_Color = vec4(color, 1.0);
}
)";

    GLuint equiProg = 0;
    {
        auto vert = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vert, 1, &equiVert, nullptr);
        glCompileShader(vert);
        auto frag = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(frag, 1, &equiFrag, nullptr);
        glCompileShader(frag);
        equiProg = glCreateProgram();
        glAttachShader(equiProg, vert);
        glAttachShader(equiProg, frag);
        glLinkProgram(equiProg);
        glDeleteShader(vert);
        glDeleteShader(frag);
    }

    // VAO for fullscreen cube
    GLuint cubeVAO, cubeVBO;
    {
        float cubeVerts[] = {
            -1, -1, -1, -1, -1, 1,  1,  -1, 1,  1,  -1, -1,  //
            -1, 1,  -1, -1, 1,  1,  1,  1,  1,  1,  1,  -1,  //
            -1, -1, -1, -1, 1,  -1, 1,  1,  -1, 1,  -1, -1,  //
            -1, -1, 1,  -1, 1,  1,  1,  1,  1,  1,  -1, 1,   //
            1,  -1, -1, 1,  1,  -1, 1,  1,  1,  1,  -1, 1,   //
            -1, -1, -1, -1, 1,  -1, -1, 1,  1,  -1, -1, 1    //
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        glBindVertexArray(cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    glViewport(0, 0, cubeSize, cubeSize);
    glUseProgram(equiProg);
    glUniformMatrix4fv(glGetUniformLocation(equiProg, "u_Projection"), 1, GL_FALSE, CUBEMAP_CAPTURE_PROJECTION);
    glUniform1i(glGetUniformLocation(equiProg, "u_EquirectangularMap"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (int i = 0; i < 6; ++i) {
        glUniformMatrix4fv(glGetUniformLocation(equiProg, "u_View"), 1, GL_FALSE, glm::value_ptr(CAPTURE_VIEWS[i]));
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                               cube->GetRendererID(), 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // Cleanup
    glDeleteProgram(equiProg);
    glDeleteTextures(1, &hdrTexture);
    glDeleteFramebuffers(1, &captureFBO);
    glDeleteRenderbuffers(1, &captureRBO);
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);

    return cube;
}

void TextureCube::Bind(uint32_t slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_rendererID);
}

void TextureCube::Unbind() const {
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
