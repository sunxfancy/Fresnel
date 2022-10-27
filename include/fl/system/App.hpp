#pragma once

#include "fl/config.hpp"
#include "fl/window/IWindow.hpp"
#include "fl/render/IRender.hpp"
#include "fl/ui/UIFramework.hpp"

namespace fl
{

class App : public IModule, public IWindowRenderCallback, public IUICallback {
public:
    App();
    virtual ~App();

    virtual void Init();
    virtual void onRender();
    virtual void onRenderUI();

    virtual void startApp();

protected:
    DEPS_ON
    sptr<UIFramework> ui;
    sptr<IRender> render;
    sptr<IWindow> window;

};

} // namespace fl

