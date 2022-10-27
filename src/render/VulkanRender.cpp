#include "fl/render/VulkanRender.hpp"
#include "fl/stdafx.hpp"
#include "fl/system/Config.hpp"

#include "VulkanImpl.hpp"

namespace fl {

VulkanRender::VulkanRender() {}

VulkanRender::~VulkanRender() {}

void VulkanRender::Init() {
    impl = new VulkanImpl();
    impl->bootstrap(window->getVulkanLoader(), window->getWindow());
    data = impl->createRenderData();
    window->setPresentCallback(this);
}

void VulkanRender::setRenderTree() { printf("set Render Tree!\n"); }


void VulkanRender::clear(glm::vec4 color) {
    impl->begin(*data);
    impl->end(*data);
}

void VulkanRender::present() {
    impl->draw_frame(*data);
}



}  // namespace fl