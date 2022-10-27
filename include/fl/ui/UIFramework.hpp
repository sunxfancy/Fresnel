#pragma once

#include "fl/config.hpp"
#include "fl/render/IRender.hpp"
#include "fl/window/IWindow.hpp"

namespace fl
{


class IUICallback {
public:
    virtual void onRenderUI() = 0;
};

struct RenderData;

class UIFramework : public IModule {
public:
    UIFramework();
    virtual ~UIFramework();

    virtual void Init();
    virtual void LoadFonts();
    virtual void setUICallback(IUICallback* cb) { uicb = cb; }
    virtual void renderUI();

protected:
    DEPS_ON
    sptr<IRender> render;
    sptr<IWindow> window;

    IUICallback* uicb = nullptr;

    virtual void initGL();
    virtual void initVulkan();
    virtual void initSDL();
    virtual void initGLFW();

    void uploadFontsForVulkan();

    RenderData* vulkan_data;
};

} // namespace fl
