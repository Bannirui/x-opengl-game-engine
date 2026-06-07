//
// Created by rui ding on 2026/3/6.
//

#pragma once

#include "x/renderer/camera/camera.h"

#include <glm/glm.hpp>

class MouseScrolledEvent;
class Event;
class Timestep;

/**
 * 透视轨迹球相机 编辑器用 自持view
 *   - 鼠标左键->绕焦点旋转
 *   - 鼠标中键->平移焦点
 *   - 鼠标滚轮/鼠标右键->缩放距离
 * 全部的操作需要按住键盘左Alt
 * 是自包含的完整相机 自己管理投影+视图+输入
 */
class EditorCamera : public Camera {
public:
    EditorCamera() = default;
    /**
     * @param fov 视角 角度
     * @param aspectRatio 宽高比
     * @param nearClip 近剪裁面
     * @param farClip 远剪裁面
     */
    EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);
    ~EditorCamera() override;

    // 每一帧都会调用过来
    void OnUpdate(Timestep ts);
    void OnEvent(Event& e);

    float get_distance() const {
        return m_distance;
    }

    /**
     * @param distance 相机到焦点的距离
     */
    void set_distance(float distance) {
        m_distance = distance;
    }

    void SetViewportSize(float width, float height) {
        m_viewportWidth = width;
        m_viewportHeight = height;
        updateProjection();
    }

    const glm::mat4& get_viewMatrix() const {
        return m_viewMatrix;
    }

    /**
     * MVP变换
     * 剪裁空间坐标
     * gl_Position = 投影矩阵*观察矩阵*模型矩阵*本地坐标
     * @return 投影矩阵*观察矩阵
     */
    glm::mat4 GetViewProjection() const {
        return m_projection * m_viewMatrix;
    }

    /**
     * @return 根据四元数计算出Up向量 是相机在世界空间的Y
     */
    glm::vec3 GetUpDirection() const;
    /**
     * @return 根据四元数计算出Right向量 是相机在世界空间的X
     */
    glm::vec3 GetRightDirection() const;
    /**
     * @return 根据四元数计算出Forward向量 是相机在世界空间的Z
     */
    glm::vec3 GetForwardDirection() const;

    const glm::vec3& get_position() const {
        return m_position;
    }

    /**
     * 四元数长什么样子 q=w+xi+yj+zk
     * 四元数的本质是什么
     * 本质是当前摄像机恣态 它编码了相机的旋转信息 后面会被用来计算Forward Right Up三个方向向量以及最终的View观察矩阵
     * 四元素最重要的最用是
     *   - 生成旋转矩阵
     *   - 续传向量
     * 为什么不用矩阵直接存 因为四元数的优点显著
     *   - 只需要4个数
     *   - 不会出现万向节锁
     *   - 插值平滑
     *   - 连续旋转误差小
     * @return 四元数
     */
    glm::quat GetOrientation() const;

    /**
     * @return 相机绕X轴的俯仰角
     */
    float get_pitch() const {
        return m_pitch;
    }

    /**
     * @return 相机绕Y轴的偏航角
     */
    float get_yaw() const {
        return m_yaw;
    }

private:
    /**
     * 影响投影矩阵的因素包括
     *   - 视野角度
     *   - 宽高比
     *   - 近距离
     *   - 远距离
     * 每一个因素的改变都会更新投影矩阵
     */
    void updateProjection();
    /**
     * 最标准的做法
     * 四元数->旋转矩阵->view matrix
     */
    void updateView();

    /**
     * @param e 鼠标滚轮滚动
     */
    bool onMouseScroll(MouseScrolledEvent& e);

    /**
     * 屏幕上的坐标 X向右是增长 Y向下是增长
     *   - 当鼠标向右滑时delta.x>0
     *   - 当鼠标向下滑时delta.y>0
     * @param delta 鼠标当前位置和上一帧的偏移量
     */
    void mousePan(glm::vec2& delta);
    /**
     * 屏幕上的坐标 X向右是增长 Y向下是增长
     *   - 当鼠标向右滑时delta.x>0
     *   - 当鼠标向下滑时delta.y>0
     * @param delta 鼠标当前位置和上一帧的偏移量
     */
    void mouseRotate(glm::vec2& delta);
    /**
     * @param delta 鼠标滚轮的前后滚动偏移值 有正负号的 滚轮向上>0 滚轮向下<0
     */
    void mouseZoom(float delta);
    /**
     * 从相机的焦点出发 沿着相机视线方向走distance就到了相机位置
     * @return 相机位置
     */
    glm::vec3 calculatePosition() const;

    /**
     * @return 在不同视口大小下对应的鼠标拖动平移速度
     */
    std::pair<float, float> panSpeed() const;
    /**
     * @return 鼠标移动灵敏度值 控制不至于鼠标移动过大
     */
    float rotationSpeed() const;
    /**
     * @return 滚轮缩放速度
     */
    float zoomSpeed() const;

private:
    // 视野FOV角度 宽高比 近平面 远平面
    float m_fov{45.0f}, m_aspectRatio{1.778f}, m_nearClip{0.1f}, m_farClip{1000.0f};
    // 视图矩阵
    glm::mat4 m_viewMatrix;
    // 相机的世界坐标 默认相机就在世界坐标原点 不会直接修改它 都是通过焦点 距离 俯仰角 偏航角变换计算出来的相机位置
    glm::vec3 m_position{0.0f};
    /**
     * 相机的焦点 相机围绕它旋转
     * 相机永远面向焦点 站在距离焦点distance处的地方面向它
     * 旋转 缩放都只会修改焦点 距离 俯仰角 偏航角
     * 然后怎么求出来相机位置呢
     * 从焦点出发 沿着相机视线的反方向走distance就是相机位置
     */
    glm::vec3 m_focalPoint{0.0f};
    // 上一帧鼠标位置 用来计算delta的
    glm::vec2 m_lastMousePosition{0.0f};

    // 相机到焦点的距离
    float m_distance{10.0f};
    // 角度 俯仰角-绕右轴X旋转 偏航角-绕世界Y轴旋转
    float m_pitch{0.0f}, m_yaw{0.0f};
    // 视窗宽度 视口高度
    float m_viewportWidth{1280.0f}, m_viewportHeight{720.0f};
};
