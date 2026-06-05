//
// Created by dingrui on 5/26/26.
//

#include "renderer/2d/renderer_2D_internal.h"
#include "x/renderer/2d/renderer_2D.h"

void Renderer2D::DrawCircle(const glm::mat4& transform, const glm::vec4& color, float thickness, float fade,
                            int entityID) {
    X_PROFILE_FUNCTION();
    for (size_t i = 0; i < 4; i++) {
        s_data.Circle.Ptr->WorldPosition = transform * s_data.QuadVertexPositions[i];
        s_data.Circle.Ptr->LocalPosition = s_data.QuadVertexPositions[i] * 2.0f;
        s_data.Circle.Ptr->Color = color;
        s_data.Circle.Ptr->Thickness = thickness;
        s_data.Circle.Ptr->Fade = fade;
        s_data.Circle.Ptr->EntityID = entityID;
        s_data.Circle.Ptr++;
    }
    // 画圆是用DrawElements的方式 用的是顶点索引 每个4个顶点画两个三角形 用了6个顶点索引来表示这两个三角形
    s_data.Circle.Count += 6;
    // 圆形绘制方式是4个顶点画2个三角形
    s_data.Stats.PrimitiveCount++;
}
