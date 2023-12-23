#pragma once


#include <vulkan/vulkan.h>
#include "vkbuilder.hpp"

#include <vector>

#include "fl/window/IWindow.hpp"

namespace fl {

constexpr int MAX_FRAMES_IN_FLIGHT = 3;

class VulkanImpl;
struct RenderData {
    VulkanImpl* parent;

    VkQueue  graphics_queue;
    VkQueue  present_queue;
    uint32_t graphics_queue_family;
    uint32_t present_queue_family;

    std::vector<VkImage>       swapchain_images;
    std::vector<VkImageView>   swapchain_image_views;
    std::vector<VkFramebuffer> framebuffers;

    VkRenderPass     render_pass;
    VkPipelineLayout pipeline_layout;
    VkPipeline       graphics_pipeline;

    VkCommandPool                command_pool;
    std::vector<VkCommandBuffer> command_buffers;

    std::vector<VkSemaphore> available_semaphores;
    std::vector<VkSemaphore> finished_semaphore;

    std::vector<VkFence> in_flight_fences;
    std::vector<VkFence> image_in_flight;

    std::vector<VkFramebuffer>   imgui_framebuffers;
    std::vector<VkCommandBuffer> imgui_command_buffers;
    VkRenderPass                 imgui_render_pass;

    size_t current_frame = 0;

    VkImage getCurrentImage() {
        return swapchain_images[current_frame];
    }


};


/**
 * @brief 
 * 
 */
class VulkanImpl {
public:
    VulkanImpl();
    ~VulkanImpl();

    vkb::Instance  instance;
    VkSurfaceKHR   surface;
    vkb::Device    device;
    vkb::Swapchain swapchain;

    vk::RenderPass renderpass;
    vk::Pipeline pipeline;

    vkb::Present present;
    
    VkDescriptorPool descriptor_pool;

    virtual RenderData* createRenderData();

    virtual void bootstrap(VulkanLoader loader, void* window);

    virtual int draw_frame(RenderData& data);
    virtual int begin(RenderData& data);
    virtual int end(RenderData& data);

    virtual int create_swapchain();
    virtual int get_queues(RenderData& data);
    virtual int create_render_pass(RenderData& data);

    virtual std::vector<uint32_t> readFile(const std::string& filename);
    virtual VkShaderModule        createShaderModule(const std::vector<uint32_t>& code);

    virtual int  create_graphics_pipeline(RenderData& data);
    virtual int  create_framebuffers(RenderData& data);
    virtual int  create_command_pool(RenderData& data);
    virtual int  create_command_buffers(RenderData& data);
    virtual int  create_sync_objects(RenderData& data);
    virtual int  recreate_swapchain(RenderData& data);
    virtual void cleanup(RenderData& data);

    virtual int create_imgui_render_pass(RenderData& data);
    virtual int create_imgui_command_buffers(RenderData& data);
    virtual int create_imgui_framebuffers(RenderData& data);

    virtual int imgui_begin(RenderData& data);
    virtual int imgui_end(RenderData& data);

    virtual void createDescriptorPool();
};

}  // namespace fl