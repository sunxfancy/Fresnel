#pragma once
#include "fl/config.hpp"

namespace fl
{

struct RenderData;

class IRender 
{
public:
    // virtual void render() = 0;
    virtual void setRenderTree() = 0;
    virtual void clear(glm::vec4 color = glm::vec4(0.2, 0.2, 0.2, 1.0)) = 0;
    virtual void present() = 0;
    // get the inner render data from vulkan, return null if not vulkan render
    virtual RenderData* getVulkanRenderData() { return nullptr; }

    virtual ~IRender() {}

    enum RenderType {
        None = 0,
        OpenGL = 1,
        Vulkan = 2,
        DirectX12 = 3,
    };

    virtual RenderType getType() = 0;
};

} // namespace genesis