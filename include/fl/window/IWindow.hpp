#pragma once

#include "fl/stdafx.hpp"


namespace fl {

class IWindowUpdateCallback {
public:
    virtual void onUpdate(float dt) = 0;
};

class IWindowRenderCallback {
public:
    virtual void onRender() = 0;
};

class IWindowKeyboardCallback {
public:
    virtual void onKeyDown(unsigned key) = 0;
    virtual void onKeyUp(unsigned key) = 0;
};

class IWindowMouseCallback {
public:
    virtual void onMouseMove(unsigned key) = 0;
    virtual void onMouseDown(unsigned key) = 0;
    virtual void onMouseUp(unsigned key) = 0;
};

class IRender;

typedef void* (* GLLoader)(const char *name);
typedef void* (* VulkanLoader)(void* instance, void* window);

class IWindow {
public:
    virtual void* getWindow() = 0;
    virtual GLLoader getGLLoader() = 0;
    virtual VulkanLoader getVulkanLoader() = 0;

    virtual void setPresentCallback(IRender* render) = 0;
    virtual void setRenderCallback(IWindowRenderCallback* callback) = 0;
    virtual void setKeyboardCallback(IWindowKeyboardCallback* callback) = 0;
    virtual void setMouseCallback(IWindowMouseCallback* callback) = 0;

    virtual void mainLoop() = 0;

    virtual ~IWindow() {}

    enum WindowType {
        None = 0,
        GLFW = 1,
        SDL2 = 2
    };

    virtual WindowType getType() = 0;
};


}