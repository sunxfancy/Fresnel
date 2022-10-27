#pragma once
#include "fl/config.hpp"
#include "fl/render/IRender.hpp"
#include "fl/window/IWindow.hpp"

namespace fl {

class VulkanImpl;
struct RenderData;

class VulkanRender : public IModule, public IRender {
public:
    VulkanRender();
    virtual ~VulkanRender();

    virtual void setRenderTree();
    virtual void clear(glm::vec4 color = glm::vec4(0.2, 0.2, 0.2, 1.0));
    virtual void present();
    
    virtual RenderType getType() { return Vulkan; }

    virtual RenderData* getVulkanRenderData() { return data; }
protected:
    DEPS_ON
    sptr<IWindow> window;

    int display_w, display_h;

    virtual void Init();

    VulkanImpl* impl;
    RenderData* data;
};

}  // namespace fl