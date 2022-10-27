#include "fl/render/VulkanImpl.hpp"

#include <fstream>
#include <array>

#include "fl/stdafx.hpp"

namespace fl {

VulkanImpl::VulkanImpl() {}

VulkanImpl::~VulkanImpl() {}

void VulkanImpl::bootstrap(VulkanLoader loader, void* window) {
    vkb::InstanceBuilder builder;

    auto inst_ret = builder
                        .require_api_version(1, 2)
                        .request_validation_layers()    // validate correctness
                        .use_default_debug_messenger()  // use DebugUtilsMessage
                        .build();
    if (!inst_ret) {
        throw std::runtime_error("Failed to create Vulkan instance. Error: " + inst_ret.error().message());
    }
    instance = inst_ret.value();
    surface  = (VkSurfaceKHR)loader(instance.instance, window);

    vkb::PhysicalDeviceSelector selector{instance};

    auto phys_ret = selector.set_surface(surface)
                        .set_minimum_version(1, 2)  // require a vulkan 1.2 capable device
                        .require_dedicated_transfer_queue()
                        .select();
    if (!phys_ret) {
        throw std::runtime_error("Failed to select Vulkan Physical Device. Error: " + phys_ret.error().message());
    }
    vkb::PhysicalDevice vbk_phys_device = phys_ret.value();

    vkb::DeviceBuilder device_builder{vbk_phys_device};

    // automatically propagate needed data from instance & physical device
    auto dev_ret = device_builder.build();
    if (!dev_ret) {
        throw std::runtime_error("Failed to create Vulkan device. Error: " + dev_ret.error().message());
    }
    device = dev_ret.value();

    createDescriptorPool();
}

void VulkanImpl::createDescriptorPool() {
    VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                                         {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                                         {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                                         {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                                         {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

    VkDescriptorPoolCreateInfo pool_info = {};

    pool_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.poolSizeCount = ((uint32_t)(sizeof(pool_sizes) / sizeof(*(pool_sizes))));
    pool_info.maxSets       = 1000 * pool_info.poolSizeCount;
    pool_info.pPoolSizes    = pool_sizes;
    VkResult err = vkCreateDescriptorPool(device.device, &pool_info, instance.allocation_callbacks, &descriptor_pool);
    if (err != VK_SUCCESS) throw std::runtime_error("Failed to create VkDescriptorPool");
}

RenderData* VulkanImpl::createRenderData() {
    RenderData* data = new RenderData();
    data->parent     = this;
    if (create_swapchain()) return nullptr;
    if (get_queues(*data)) return nullptr;
    if (create_render_pass(*data)) return nullptr;
    if (create_graphics_pipeline(*data)) return nullptr;
    if (create_framebuffers(*data)) return nullptr;
    if (create_command_pool(*data)) return nullptr;
    if (create_command_buffers(*data)) return nullptr;
    if (create_sync_objects(*data)) return nullptr;
    if (create_imgui_render_pass(*data)) return nullptr;
    if (create_imgui_framebuffers(*data)) return nullptr;
    if (create_imgui_command_buffers(*data)) return nullptr;
    return data;
}

int VulkanImpl::create_swapchain() {
    vkb::SwapchainBuilder swapchain_builder{device};
    auto                  swap_ret = swapchain_builder.set_old_swapchain(swapchain).build();
    if (!swap_ret) {
        std::cout << swap_ret.error().message() << " " << swap_ret.vk_result() << "\n";
        return -1;
    }
    vkb::destroy_swapchain(swapchain);
    swapchain = swap_ret.value();
    return 0;
}

int VulkanImpl::get_queues(RenderData& data) {
    auto gqi = device.get_queue_index(vkb::QueueType::graphics);
    if (!gqi.has_value()) {
        std::cout << "failed to get graphics queue: " << gqi.error().message() << "\n";
        return -1;
    }
    auto gq                    = device.get_queue(vkb::QueueType::graphics);
    data.graphics_queue        = gq.value();
    data.graphics_queue_family = gqi.value();

    auto pqi = device.get_queue_index(vkb::QueueType::present);
    if (!pqi.has_value()) {
        std::cout << "failed to get present queue: " << pqi.error().message() << "\n";
        return -1;
    }
    auto pq                   = device.get_queue(vkb::QueueType::present);
    data.present_queue        = pq.value();
    data.present_queue_family = pqi.value();
    return 0;
}

int VulkanImpl::create_imgui_render_pass(RenderData& data) {
    // To create this render pass we first need to create a VkAttachmentDescription. Let’s get rid of the boring fields
    // first: we don’t need any stencil so we don’t care about its operators and the number of samples should probably
    // be 1. Dear ImGui’s output looks fine without MSAA. The format used depends on your swapchain and you should be
    // able to find the relevant VkFormat somewhere in your code. It is usually extracted from the
    // SwapChainSupportDetails structure returned by the function querySwapChainSupport.

    // Ok, so two of the most relevant parts here are loadOp and initialLayout. The first one should be
    // VK_ATTACHMENT_LOAD_OP_LOAD because you want your GUI to be drawn over your main rendering. This tells Vulkan you
    // don’t want to clear the content of the framebuffer but you want to draw over it instead. Since we’re going to
    // draw some stuff, we also want initialLayout to be set to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL for optimal
    // performance. And because this render pass is the last one, we want finalLayout to be set to
    // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR. This will automatically transition our attachment to the right layout for
    // presentation.
    VkAttachmentDescription attachment = {};
    attachment.format                  = swapchain.image_format;
    attachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.finalLayout             = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Now that we have this VkAttachmentDescription we can create the actual color VkAttachmentReference that our
    // render pass needs. As we described above, the layout we’re using to draw is
    // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL.

    VkAttachmentReference color_attachment = {};
    color_attachment.attachment            = 0;
    color_attachment.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // We can now create a subpass for our render pass using the previously created attachment. It is obviously a
    // graphics subpass.

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &color_attachment;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass          = 0;
    dependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask       = 0;  // or VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo info = {};
    info.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount        = 1;
    info.pAttachments           = &attachment;
    info.subpassCount           = 1;
    info.pSubpasses             = &subpass;
    info.dependencyCount        = 1;
    info.pDependencies          = &dependency;
    if (vkCreateRenderPass(device.device, &info, nullptr, &data.imgui_render_pass) != VK_SUCCESS) {
        throw std::runtime_error("Could not create Dear ImGui's render pass");
    }

    return 0;
}

int VulkanImpl::create_imgui_command_buffers(RenderData& data) {
    data.imgui_command_buffers.resize(data.swapchain_image_views.size());

    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandPool                 = data.command_pool;
    commandBufferAllocateInfo.commandBufferCount          = data.imgui_command_buffers.size();
    vkAllocateCommandBuffers(device.device, &commandBufferAllocateInfo, data.imgui_command_buffers.data());

    return 0;
}

int VulkanImpl::create_imgui_framebuffers(RenderData& data) {
    data.swapchain_images      = swapchain.get_images().value();
    data.swapchain_image_views = swapchain.get_image_views().value();

    data.imgui_framebuffers.resize(data.swapchain_image_views.size());

    for (size_t i = 0; i < data.swapchain_image_views.size(); i++) {
        VkImageView attachments[] = {data.swapchain_image_views[i]};

        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass              = data.imgui_render_pass;
        framebuffer_info.attachmentCount         = 1;
        framebuffer_info.pAttachments            = attachments;
        framebuffer_info.width                   = swapchain.extent.width;
        framebuffer_info.height                  = swapchain.extent.height;
        framebuffer_info.layers                  = 1;

        if (vkCreateFramebuffer(device.device, &framebuffer_info, nullptr, &data.imgui_framebuffers[i]) != VK_SUCCESS) {
            return -1;  // failed to create framebuffer
        }
    }
    return 0;
}

int VulkanImpl::create_render_pass(RenderData& data) {
    VkAttachmentDescription color_attachment = {};
    color_attachment.format                  = swapchain.image_format;
    color_attachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout             = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference color_attachment_ref = {};
    color_attachment_ref.attachment            = 0;
    color_attachment_ref.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &color_attachment_ref;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass          = 0;
    dependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask       = 0;
    dependency.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount        = 1;
    render_pass_info.pAttachments           = &color_attachment;
    render_pass_info.subpassCount           = 1;
    render_pass_info.pSubpasses             = &subpass;
    render_pass_info.dependencyCount        = 1;
    render_pass_info.pDependencies          = &dependency;

    if (vkCreateRenderPass(device.device, &render_pass_info, nullptr, &data.render_pass) != VK_SUCCESS) {
        std::cout << "failed to create render pass\n";
        return -1;  // failed to create render pass!
    }
    return 0;
}

std::vector<uint32_t> VulkanImpl::readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t file_size = (size_t)file.tellg();

    std::vector<uint32_t> buffer((file_size + 3) / 4);
    file.seekg(0);
    file.read((char*)buffer.data(), static_cast<std::streamsize>(file_size));
    file.close();

    return buffer;
}

VkShaderModule VulkanImpl::createShaderModule(const std::vector<uint32_t>& code) {
    VkShaderModuleCreateInfo create_info = {};

    create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size() * 4;
    create_info.pCode    = code.data();

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device.device, &create_info, nullptr, &shaderModule) != VK_SUCCESS) {
        return VK_NULL_HANDLE;  // failed to create shader module
    }

    return shaderModule;
}

int VulkanImpl::create_graphics_pipeline(RenderData& data) {
    auto vert_code = readFile("vert.spv");
    auto frag_code = readFile("frag.spv");

    VkShaderModule vert_module = createShaderModule(vert_code);
    VkShaderModule frag_module = createShaderModule(frag_code);
    if (vert_module == VK_NULL_HANDLE || frag_module == VK_NULL_HANDLE) {
        std::cout << "failed to create shader module\n";
        return -1;  // failed to create shader modules
    }

    VkPipelineShaderStageCreateInfo vert_stage_info = {};

    vert_stage_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_stage_info.stage  = VK_SHADER_STAGE_VERTEX_BIT;
    vert_stage_info.module = vert_module;
    vert_stage_info.pName  = "main";

    VkPipelineShaderStageCreateInfo frag_stage_info = {};

    frag_stage_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_stage_info.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_stage_info.module = frag_module;
    frag_stage_info.pName  = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = {vert_stage_info, frag_stage_info};

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};

    vertex_input_info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount   = 0;
    vertex_input_info.vertexAttributeDescriptionCount = 0;

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {};

    input_assembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x          = 0.0f;
    viewport.y          = 0.0f;
    viewport.width      = (float)swapchain.extent.width;
    viewport.height     = (float)swapchain.extent.height;
    viewport.minDepth   = 0.0f;
    viewport.maxDepth   = 1.0f;

    VkRect2D scissor = {};
    scissor.offset   = {0, 0};
    scissor.extent   = swapchain.extent;

    VkPipelineViewportStateCreateInfo viewport_state = {};

    viewport_state.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports    = &viewport;
    viewport_state.scissorCount  = 1;
    viewport_state.pScissors     = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};

    rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable        = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth               = 1.0f;
    rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace               = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable         = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};

    multisampling.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable  = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo color_blending = {};

    color_blending.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable     = VK_FALSE;
    color_blending.logicOp           = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount   = 1;
    color_blending.pAttachments      = &colorBlendAttachment;
    color_blending.blendConstants[0] = 0.0f;
    color_blending.blendConstants[1] = 0.0f;
    color_blending.blendConstants[2] = 0.0f;
    color_blending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipeline_layout_info = {};

    pipeline_layout_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount         = 0;
    pipeline_layout_info.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(device.device, &pipeline_layout_info, nullptr, &data.pipeline_layout) != VK_SUCCESS) {
        std::cout << "failed to create pipeline layout\n";
        return -1;  // failed to create pipeline layout
    }

    std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamic_info = {};

    dynamic_info.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamic_info.pDynamicStates    = dynamic_states.data();

    VkGraphicsPipelineCreateInfo pipeline_info = {};

    pipeline_info.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount          = 2;
    pipeline_info.pStages             = shader_stages;
    pipeline_info.pVertexInputState   = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState      = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState   = &multisampling;
    pipeline_info.pColorBlendState    = &color_blending;
    pipeline_info.pDynamicState       = &dynamic_info;
    pipeline_info.layout              = data.pipeline_layout;
    pipeline_info.renderPass          = data.render_pass;
    pipeline_info.subpass             = 0;
    pipeline_info.basePipelineHandle  = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(device.device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &data.graphics_pipeline) !=
        VK_SUCCESS) {
        std::cout << "failed to create pipline\n";
        return -1;  // failed to create graphics pipeline
    }

    vkDestroyShaderModule(device.device, frag_module, nullptr);
    vkDestroyShaderModule(device.device, vert_module, nullptr);
    return 0;
}

int VulkanImpl::create_framebuffers(RenderData& data) {
    data.swapchain_images      = swapchain.get_images().value();
    data.swapchain_image_views = swapchain.get_image_views().value();

    data.framebuffers.resize(data.swapchain_image_views.size());

    for (size_t i = 0; i < data.swapchain_image_views.size(); i++) {
        VkImageView attachments[] = {data.swapchain_image_views[i]};

        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass              = data.render_pass;
        framebuffer_info.attachmentCount         = 1;
        framebuffer_info.pAttachments            = attachments;
        framebuffer_info.width                   = swapchain.extent.width;
        framebuffer_info.height                  = swapchain.extent.height;
        framebuffer_info.layers                  = 1;

        if (vkCreateFramebuffer(device.device, &framebuffer_info, nullptr, &data.framebuffers[i]) != VK_SUCCESS) {
            return -1;  // failed to create framebuffer
        }
    }
    return 0;
}

int VulkanImpl::create_command_pool(RenderData& data) {
    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex        = device.get_queue_index(vkb::QueueType::graphics).value();
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if (vkCreateCommandPool(device.device, &pool_info, nullptr, &data.command_pool) != VK_SUCCESS) {
        std::cout << "failed to create command pool\n";
        return -1;  // failed to create command pool
    }
    return 0;
}

int VulkanImpl::create_command_buffers(RenderData& data) {
    data.command_buffers.resize(data.framebuffers.size());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool                 = data.command_pool;
    allocInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount          = (uint32_t)data.command_buffers.size();

    if (vkAllocateCommandBuffers(device.device, &allocInfo, data.command_buffers.data()) != VK_SUCCESS) {
        return -1;  // failed to allocate command buffers;
    }

    return 0;
}

int VulkanImpl::begin(RenderData& data) {
    auto i = data.current_frame;
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(data.command_buffers[i], &begin_info) != VK_SUCCESS) {
        return -1;  // failed to begin recording command buffer
    }

    VkRenderPassBeginInfo render_pass_info = {};
    render_pass_info.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass            = data.render_pass;
    render_pass_info.framebuffer           = data.framebuffers[i];
    render_pass_info.renderArea.offset     = {0, 0};
    render_pass_info.renderArea.extent     = swapchain.extent;
    VkClearValue clearColor{{{0.0f, 0.0f, 0.0f, 1.0f}}};
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues    = &clearColor;

    VkViewport viewport = {};
    viewport.x          = 0.0f;
    viewport.y          = 0.0f;
    viewport.width      = (float)swapchain.extent.width;
    viewport.height     = (float)swapchain.extent.height;
    viewport.minDepth   = 0.0f;
    viewport.maxDepth   = 1.0f;

    VkRect2D scissor = {};
    scissor.offset   = {0, 0};
    scissor.extent   = swapchain.extent;

    vkCmdSetViewport(data.command_buffers[i], 0, 1, &viewport);
    vkCmdSetScissor(data.command_buffers[i], 0, 1, &scissor);

    vkCmdBeginRenderPass(data.command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(data.command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, data.graphics_pipeline);
    vkCmdDraw(data.command_buffers[i], 3, 1, 0, 0);

    return 0;
}

int VulkanImpl::end(RenderData& data) {
    auto i = data.current_frame;
    vkCmdEndRenderPass(data.command_buffers[i]);

    if (vkEndCommandBuffer(data.command_buffers[i]) != VK_SUCCESS) {
        std::cout << "failed to record command buffer\n";
        return -1;  // failed to record command buffer!
    }
    return 0;
}

int VulkanImpl::create_sync_objects(RenderData& data) {
    data.available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    data.finished_semaphore.resize(MAX_FRAMES_IN_FLIGHT);
    data.in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
    data.image_in_flight.resize(swapchain.image_count, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {};
    fence_info.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags             = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device.device, &semaphore_info, nullptr, &data.available_semaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device.device, &semaphore_info, nullptr, &data.finished_semaphore[i]) != VK_SUCCESS ||
            vkCreateFence(device.device, &fence_info, nullptr, &data.in_flight_fences[i]) != VK_SUCCESS) {
            std::cout << "failed to create sync objects\n";
            return -1;  // failed to create synchronization objects for a frame
        }
    }
    return 0;
}

int VulkanImpl::recreate_swapchain(RenderData& data) {
    vkDeviceWaitIdle(device.device);

    vkDestroyCommandPool(device.device, data.command_pool, nullptr);

    for (auto framebuffer : data.framebuffers) {
        vkDestroyFramebuffer(device.device, framebuffer, nullptr);
    }

    swapchain.destroy_image_views(data.swapchain_image_views);

    if (0 != create_swapchain()) return -1;
    if (0 != create_framebuffers(data)) return -1;
    if (0 != create_command_pool(data)) return -1;
    if (0 != create_command_buffers(data)) return -1;
    return 0;
}

int VulkanImpl::imgui_begin(RenderData& data) {
    VkResult                 err;
    VkCommandBufferBeginInfo binfo = {};
    binfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    // binfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    err = vkBeginCommandBuffer(data.imgui_command_buffers[data.current_frame], &binfo);

    VkRenderPassBeginInfo info = {};
    info.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.renderPass            = data.imgui_render_pass;
    info.framebuffer           = data.imgui_framebuffers[data.current_frame];
    info.renderArea.offset     = {0, 0};
    info.renderArea.extent     = swapchain.extent;
    info.clearValueCount       = 1;
    VkClearValue clearColor{{{0.0f, 0.0f, 0.0f, 1.0f}}};
    info.pClearValues = &clearColor;
    vkCmdBeginRenderPass(data.imgui_command_buffers[data.current_frame], &info, VK_SUBPASS_CONTENTS_INLINE);
    return 0;
}

int VulkanImpl::imgui_end(RenderData& data) {
    
    // Submit command buffer
    vkCmdEndRenderPass(data.imgui_command_buffers[data.current_frame]);
    vkEndCommandBuffer(data.imgui_command_buffers[data.current_frame]);
    return 0;
}

int VulkanImpl::draw_frame(RenderData& data) {
    vkWaitForFences(device.device, 1, &data.in_flight_fences[data.current_frame], VK_TRUE, UINT64_MAX);

    uint32_t image_index = 0;
    VkResult result =
        vkAcquireNextImageKHR(device.device, swapchain.swapchain, UINT64_MAX,
                              data.available_semaphores[data.current_frame], VK_NULL_HANDLE, &image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        return recreate_swapchain(data);
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        std::cout << "failed to acquire swapchain image. Error " << result << "\n";
        return -1;
    }

    if (data.image_in_flight[image_index] != VK_NULL_HANDLE) {
        vkWaitForFences(device.device, 1, &data.image_in_flight[image_index], VK_TRUE, UINT64_MAX);
    }
    data.image_in_flight[image_index] = data.in_flight_fences[data.current_frame];


    std::array<VkCommandBuffer, 2> submitCommandBuffers = 
    { data.command_buffers[image_index], data.imgui_command_buffers[image_index] };


    VkSubmitInfo submitInfo = {};
    submitInfo.sType        = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore          wait_semaphores[] = {data.available_semaphores[data.current_frame]};
    VkPipelineStageFlags wait_stages[]     = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount          = 1;
    submitInfo.pWaitSemaphores             = wait_semaphores;
    submitInfo.pWaitDstStageMask           = wait_stages;

    submitInfo.commandBufferCount = 2;
    submitInfo.pCommandBuffers    = submitCommandBuffers.data();

    VkSemaphore signal_semaphores[] = {data.finished_semaphore[data.current_frame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = signal_semaphores;

    vkResetFences(device.device, 1, &data.in_flight_fences[data.current_frame]);

    if (vkQueueSubmit(data.graphics_queue, 1, &submitInfo, data.in_flight_fences[data.current_frame]) != VK_SUCCESS) {
        std::cout << "failed to submit draw command buffer\n";
        return -1;  //"failed to submit draw command buffer
    }

    vkDeviceWaitIdle(device.device);

    VkPresentInfoKHR present_info = {};
    present_info.sType            = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores    = signal_semaphores;

    VkSwapchainKHR swapChains[] = {swapchain.swapchain};
    present_info.swapchainCount = 1;
    present_info.pSwapchains    = swapChains;

    present_info.pImageIndices = &image_index;

    result = vkQueuePresentKHR(data.present_queue, &present_info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        return recreate_swapchain(data);
    } else if (result != VK_SUCCESS) {
        std::cout << "failed to present swapchain image\n";
        return -1;
    }

    data.current_frame = (data.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
    return 0;
}

void VulkanImpl::cleanup(RenderData& data) {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device.device, data.finished_semaphore[i], nullptr);
        vkDestroySemaphore(device.device, data.available_semaphores[i], nullptr);
        vkDestroyFence(device.device, data.in_flight_fences[i], nullptr);
    }

    vkDestroyCommandPool(device.device, data.command_pool, nullptr);

    for (auto framebuffer : data.framebuffers) {
        vkDestroyFramebuffer(device.device, framebuffer, nullptr);
    }

    vkDestroyPipeline(device.device, data.graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(device.device, data.pipeline_layout, nullptr);
    vkDestroyRenderPass(device.device, data.render_pass, nullptr);

    swapchain.destroy_image_views(data.swapchain_image_views);

    vkb::destroy_swapchain(swapchain);
    vkb::destroy_device(device);
    vkDestroySurfaceKHR(instance.instance, surface, nullptr);
    vkb::destroy_instance(instance);
}

}  // namespace fl