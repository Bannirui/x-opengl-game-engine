//
// Created by rui ding on 2026/2/24.
//

#include "platform/mac/mac_window.h"

#include "platform/opengl/open_gl_context.h"
#include "x/x_log.h"
#include "x/events/application_event.h"
#include "x/events/key_event.h"
#include "x/events/mouse_event.h"
#include "platform/opengl/open_gl_context.h"

static bool s_GLFWInitialized = false;

static void glfwErrorCallback(int error, const char *description) {
    X_CORE_ERROR("GLFW error ({0}): {1}", error, description);
}

MacWindow::MacWindow(const WindowProps &props) {
    init(props);
}

MacWindow::~MacWindow() {
    shutdown();
}

void MacWindow::OnUpdate() {
    glfwPollEvents();
    glfwSwapBuffers(m_window);
}

void MacWindow::SetVSync(bool enabled) {
    if (enabled) {
        glfwSwapInterval(1);
    } else {
        glfwSwapInterval(0);
    }
    m_data.vSync = enabled;
}

void MacWindow::init(const WindowProps &props) {
    m_data.title = props.title;
    m_data.width = props.width;
    m_data.height = props.height;

    X_CORE_INFO("Create window {0} ({1}, {2})", props.title, props.width, props.height);

    if (!s_GLFWInitialized) {
        int succ = glfwInit();
        X_CORE_ASSERT(succ, "Could not init GLFW");
        glfwSetErrorCallback(glfwErrorCallback);
        s_GLFWInitialized = true;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, X_GL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, X_GL_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    m_window = glfwCreateWindow(static_cast<int>(props.width), static_cast<int>(props.height), props.title.c_str(),
                                nullptr, nullptr);
    X_CORE_ASSERT(m_window, "Failed to create GLFW window");

    glfwMakeContextCurrent(m_window);

    m_context = new OpenGLContext(m_window);
    m_context->Init();

    glfwSetWindowUserPointer(m_window, this);

    SetVSync(true);
    // glfw的回调注册
    glfwSetWindowSizeCallback(m_window,
                              [](GLFWwindow *window, int width, int height) {
                                  auto *self = static_cast<MacWindow *>(glfwGetWindowUserPointer(window));
                                  self->m_data.width = width;
                                  self->m_data.height = height;
                                  WindowResizeEvent event(width, height);
                                  self->m_data.eventCallback(event);
                              });
    glfwSetWindowCloseCallback(m_window,
                               [](GLFWwindow *window) {
                                   auto *self = static_cast<MacWindow *>(glfwGetWindowUserPointer(window));
                                   WindowCloseEvent event;
                                   self->m_data.eventCallback(event);
                               });
    glfwSetKeyCallback(m_window,
                       [](GLFWwindow *window, int key, int scancode, int action, int mods) {
                           auto *self = static_cast<MacWindow *>(glfwGetWindowUserPointer(window));
                           switch (action) {
                               case GLFW_PRESS: {
                                   KeyPressEvent event(key, false);
                                   self->m_data.eventCallback(event);
                                   break;
                               }
                               case GLFW_RELEASE: {
                                   KeyReleaseEvent event(key);
                                   self->m_data.eventCallback(event);
                                   break;
                               }
                               case GLFW_REPEAT: {
                                   KeyPressEvent event(key, true);
                                   self->m_data.eventCallback(event);
                                   break;
                               }
                           }
                       });
    glfwSetCharCallback(m_window,
                        [](GLFWwindow *window, uint32_t keycode) {
                            auto *self = static_cast<MacWindow *>(glfwGetWindowUserPointer(window));
                            KeyTypedEvent event(keycode);
                            self->m_data.eventCallback(event);
                        });
    glfwSetMouseButtonCallback(m_window,
                               [](GLFWwindow *window, int button, int action, int mods) {
                                   auto *self = static_cast<MacWindow *>(glfwGetWindowUserPointer(window));
                                   switch (action) {
                                       case GLFW_PRESS: {
                                           MouseButtonPressedEvent event(button);
                                           self->m_data.eventCallback(event);
                                           break;
                                       }
                                       case GLFW_RELEASE: {
                                           MouseButtonReleasedEvent event(button);
                                           self->m_data.eventCallback(event);
                                           break;
                                       }
                                   }
                               });
    glfwSetScrollCallback(m_window,
                          [](GLFWwindow *window, double xOffset, double yOffset) {
                              auto *self = static_cast<MacWindow *>(glfwGetWindowUserPointer(window));
                              MouseScrolledEvent event(xOffset, yOffset);
                              self->m_data.eventCallback(event);
                          });
    glfwSetCursorPosCallback(m_window,
                             [](GLFWwindow *window, double xPos, double yPos) {
                                 auto *self = static_cast<MacWindow *>(glfwGetWindowUserPointer(window));
                                 MouseMovedEvent event(xPos, yPos);
                                 self->m_data.eventCallback(event);
                             });
}

void MacWindow::shutdown() {
    glfwDestroyWindow(m_window);
}
