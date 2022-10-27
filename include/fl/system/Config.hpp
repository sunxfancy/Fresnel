#pragma once

#include "fl/config.hpp"
#include "fl/window/IWindow.hpp"

namespace fl {
constexpr int32_t autocenter = 1009800898;

class Config : public IModule {
public:
    uint32_t width, height;
    int32_t  x, y;  // window position when starting, if the value is autocenter, then it will choose center position
                    // automatically

    bool full_screen;  // window is opened in fullscreen mode
    bool hidden;       // window is hidden when starting

    bool allow_high_DPI;  // allow high DPI on Windows and MacOSX, the screen will be larger than width and height

    bool borderless;  // create a borderless window
    bool minimized;   // window minimized when starting
    bool maximized;   // window maximized when starting
    bool resizable;   // window can resize by dragging

    bool mouse_capture;  // mouse captured in the window, usually for shotting games
    bool always_on_top;  // always on top of the screen

    bool enable_vsync;              // keep 60fps same as your monitor
    bool enable_default_keyaction;  // enable default handle for ESC to exit app

    bool enable_vulkan_support;
    bool enable_opengl_support;

    std::string window_title;

    uint32_t OpenGL_Version_Major;
    uint32_t OpenGL_Version_Minor;

    Config();
    ~Config() {}
};

};  // namespace fl