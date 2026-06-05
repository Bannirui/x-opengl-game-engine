//
// Created by dingrui on 5/26/26.
//

#include "renderer/2d/renderer_2D_internal.h"
#include "x/renderer/2d/renderer_2D.h"

void Renderer2D::DrawLine(const glm::vec3& p0, glm::vec3& p1, const glm::vec4& color, int entityID) {
    s_data.Line.Ptr->Position = p0;
    s_data.Line.Ptr->Color = color;
    s_data.Line.Ptr->EntityID = entityID;
    s_data.Line.Ptr++;

    s_data.Line.Ptr->Position = p1;
    s_data.Line.Ptr->Color = color;
    s_data.Line.Ptr->EntityID = entityID;
    s_data.Line.Ptr++;

    s_data.Line.Count += 2;
}

void Renderer2D::DrawRect(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, int entityID) {
    glm::vec3 p0 = glm::vec3(position.x - size.x * 0.5f, position.y - size.y * 0.5f, position.z);
    glm::vec3 p1 = glm::vec3(position.x + size.x * 0.5f, position.y - size.y * 0.5f, position.z);
    glm::vec3 p2 = glm::vec3(position.x + size.x * 0.5f, position.y + size.y * 0.5f, position.z);
    glm::vec3 p3 = glm::vec3(position.x - size.x * 0.5f, position.y + size.y * 0.5f, position.z);
    DrawLine(p0, p1, color, entityID);
    DrawLine(p1, p2, color, entityID);
    DrawLine(p2, p3, color, entityID);
    DrawLine(p3, p0, color, entityID);
}

void Renderer2D::DrawRect(const glm::mat4& transform, const glm::vec4& color, int entityID) {
    glm::vec3 lineVertices[4];
    for (size_t i = 0; i < 4; ++i) {
        lineVertices[i] = transform * s_data.QuadVertexPositions[i];
    }
    DrawLine(lineVertices[0], lineVertices[1], color, entityID);
    DrawLine(lineVertices[1], lineVertices[2], color, entityID);
    DrawLine(lineVertices[2], lineVertices[3], color, entityID);
    DrawLine(lineVertices[3], lineVertices[0], color, entityID);
}

float Renderer2D::GetLineWidth() {
    return s_data.LineWidth;
}

void Renderer2D::SetLineWidth(float width) {
    s_data.LineWidth = width;
}
