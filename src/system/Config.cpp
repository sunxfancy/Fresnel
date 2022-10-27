
#include "fl/system/Config.hpp"

namespace fl
{
    
Config::Config()
{
    width = 1920; height = 1080;
    x = autocenter; y = autocenter;
    full_screen = false;
    hidden = false;
    allow_high_DPI = true;
    borderless = false;
    minimized = false;
    maximized = false;
    resizable = false;
    mouse_capture = false;
    always_on_top = false;
    window_title = "Fresnel";

    enable_vsync = false;
    OpenGL_Version_Major = 4;
    OpenGL_Version_Minor = 3;
    enable_opengl_support = false;
    enable_vulkan_support = true;
}


} // namespace fl
