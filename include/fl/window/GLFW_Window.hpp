#pragma once

#include "fl/config.hpp"
#include "fl/window/IWindow.hpp"
#include "fl/system/Config.hpp"

struct GLFWwindow;

namespace fl {

class IRender;

class GLFW_Window : public IModule, public IWindow {
public:
    GLFW_Window();
    virtual ~GLFW_Window();

    virtual void mainLoop();

    virtual void setPresentCallback(IRender* render);
    virtual void setRenderCallback(IWindowRenderCallback* callback);
    virtual void setKeyboardCallback(IWindowKeyboardCallback* callback);
    virtual void setMouseCallback(IWindowMouseCallback* callback);

    virtual void*    getWindow();
    virtual GLLoader getGLLoader();
    virtual VulkanLoader getVulkanLoader();

    virtual WindowType getType() { return GLFW; }
protected:
    DEPS_ON
    sptr<Config> config;

    struct GLFWwindow* window;
    bool enable_gl = false;
    bool enable_vulkan = false;

    virtual void Init();
    virtual void createWindow();
    virtual void swapBuffers();
    virtual void cleanUp();
    virtual void processInput();

    IRender* render = nullptr;
    IWindowRenderCallback*   render_cb   = nullptr;
    IWindowKeyboardCallback* keyboard_cb = nullptr;
    IWindowMouseCallback*    mouse_cb    = nullptr;
};

}  // namespace fl
