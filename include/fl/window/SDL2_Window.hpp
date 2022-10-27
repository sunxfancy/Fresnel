#pragma once

#include "fl/config.hpp"
#include "fl/window/IWindow.hpp"
#include "fl/system/Config.hpp"

struct SDL_Window;

namespace fl {

class IRender;

class SDL2_Window : public IModule, public IWindow {
public:
    SDL2_Window();
    virtual ~SDL2_Window();

    virtual void mainLoop();

    virtual void setPresentCallback(IRender* render);
    virtual void setRenderCallback(IWindowRenderCallback* callback);
    virtual void setKeyboardCallback(IWindowKeyboardCallback* callback);
    virtual void setMouseCallback(IWindowMouseCallback* callback);

    virtual void*    getWindow();
    virtual GLLoader getGLLoader();
    virtual VulkanLoader getVulkanLoader();

    virtual WindowType getType() { return SDL2; }
protected:
    DEPS_ON
    sptr<Config> config;

    SDL_Window* window;
    bool done = false;
    
    virtual void Init();
    virtual void createWindow();
    virtual void cleanUp();
    virtual void processInput();

    virtual uint32_t createFlags();

    IRender* render = nullptr;
    IWindowRenderCallback*   render_cb   = nullptr;
    IWindowKeyboardCallback* keyboard_cb = nullptr;
    IWindowMouseCallback*    mouse_cb    = nullptr;
};

}  // namespace fl
