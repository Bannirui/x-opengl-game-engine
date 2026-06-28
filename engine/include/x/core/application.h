//
// Created by dingrui on 2/23/26.
//

#pragma once

#include "x/core/assert.h"
#include "x/core/base.h"
#include "x/layer_stack.h"

#include <queue>

class WindowResizeEvent;
class ImGuiLayer;
class Window;
class Event;
class WindowCloseEvent;
class Layer;

int main(int argc, char** argv);

struct ApplicationCommandLineArgs {
    int Count = 0;
    char** Args = nullptr;

    const char* operator[](int index) const {
        X_CORE_ASSERT(index < Count, "Index out of bounds, index={}, Count={}", index, Count);
        return Args[index];
    }
};

struct ApplicationSpecification {
    std::string Name = "X Application";
    std::string WorkingDirectory;
    ApplicationCommandLineArgs CommandLineArgs;
};

class XApplication {
public:
    XApplication(const ApplicationSpecification& specification);
    virtual ~XApplication();

    /**
     * 接收glfw的窗体事件 对它们进行处理
     * @param e 收到的窗体事件
     */
    void OnEvent(Event& e);

    void PushLayer(Scope<Layer> layer);
    void PushOverlay(Scope<Layer> overlay);

    void Close();

    Window& get_window() const {
        return *m_window;
    }

    ImGuiLayer* get_ImGuiLayer() const {
        return m_ImGuiLayer;
    }

    static XApplication& Get() {
        return *s_instance;
    }

    ApplicationSpecification get_specification() const {
        return m_specification;
    }

private:
    void run();
    /**
     * 在run的循环里面 每个帧一起处理缓存的事件队列
     */
    void ProcessEvents();

private:
    ApplicationSpecification m_specification;
    Scope<Window> m_window;
    bool m_running{true};
    LayerStack m_layerStack;
    ImGuiLayer* m_ImGuiLayer;
    float m_lastFrameTime{0.0f};
    bool m_minimized{false};  // 窗口最小化
    // 事件队列 把收到的回调事件缓存着延迟批量处理
    std::queue<std::unique_ptr<Event>> m_eventQueue;

private:
    static XApplication* s_instance;
    friend int main(int argc, char** argv);
};

// To be defined in the CLIENT
XApplication* CreateApplication(ApplicationCommandLineArgs args);
