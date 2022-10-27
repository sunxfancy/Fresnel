
#include "fl/render/VulkanRender.hpp"
#include "fl/system/App.hpp"
#include "fl/system/Config.hpp"
#include "fl/window/GLFW_Window.hpp"

using namespace fl;

class RenderCallback : public IWindowRenderCallback {
public:
    RenderCallback(sptr<IRender> render) : render(render) {}
    virtual void onRender() {
        render->clear();
    }

    sptr<IRender> render;
};

void configDI(DI& di) {
    di["IWindow"]     = []() -> IModule* { return new GLFW_Window(); };
    di["IRender"]     = []() -> IModule* { return new VulkanRender(); };
    di["Config"]      = []() -> IModule* { auto p = new Config(); p->enable_vulkan_support = true; return p; };
}

int main() {
    DI di;
    configDI(di);
    sptr<IRender> render = di.get<IRender>("IRender");
    sptr<IWindow> window = di.get<IWindow>("IWindow");
    RenderCallback* cb = new RenderCallback(render);
    window->setRenderCallback(cb);
    window->mainLoop();
    return 0;
}