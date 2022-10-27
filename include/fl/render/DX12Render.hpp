#pragma once
#include "fl/config.hpp"
#include "fl/render/IRender.hpp"
#include "fl/window/IWindow.hpp"

class DX12Render : public IModule, public IRender {
public:
    DX12Render();
    virtual ~DX12Render();

    virtual void setRenderTree();
    virtual void clear(glm::vec4 color = glm::vec4(0.2, 0.2, 0.2, 1.0));
    virtual void present() {}
    virtual RenderType getType() { return DirectX12; }
protected:
    DEPS_ON
    sptr<IWindow> window;

    bool show_demo_window    = true;
    bool show_another_window = false;

    int display_w, display_h;

    virtual void Init();
    virtual void initGL();
};