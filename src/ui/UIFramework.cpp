#include "fl/ui/UIFramework.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_vulkan.h>

#include "fl/render/VulkanImpl.hpp"

namespace fl {

UIFramework::UIFramework() {}
UIFramework::~UIFramework() {}

void UIFramework::Init() {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // ImGuiIO& io = ImGui::GetIO();
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    if (window->getType() == IWindow::WindowType::GLFW) {
        initGLFW();
    } else if (window->getType() == IWindow::WindowType::SDL2) {
        initSDL();
    }
    if (render->getType() == IRender::RenderType::OpenGL) {
        initGL();
    } else if (render->getType() == IRender::RenderType::Vulkan) {
        initVulkan();
    }
}

void UIFramework::LoadFonts() {
    // ImGuiIO& io = ImGui::GetIO();
    // io.Fonts->AddFontDefault();
    ImGuiIO& io = ImGui::GetIO();
    ImFont* font = io.Fonts->AddFontFromFileTTF("../data/fonts/msyhl.ttc", 18.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
    IM_ASSERT(font != NULL);
}

void UIFramework::renderUI() {
    if (render->getType() == IRender::RenderType::OpenGL) ImGui_ImplOpenGL3_NewFrame();
    if (render->getType() == IRender::RenderType::Vulkan) ImGui_ImplVulkan_NewFrame();
    if (window->getType() == IWindow::WindowType::GLFW) ImGui_ImplGlfw_NewFrame();
    if (window->getType() == IWindow::WindowType::SDL2) ImGui_ImplSDL2_NewFrame((SDL_Window*)(window->getWindow()));

    ImGui::NewFrame();
    if (uicb) uicb->onRenderUI();
    ImGui::Render();

    if (render->getType() == IRender::RenderType::OpenGL) ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    if (render->getType() == IRender::RenderType::Vulkan) {
        vulkan_data->parent->imgui_begin(*vulkan_data);
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
                                        vulkan_data->imgui_command_buffers[vulkan_data->current_frame]);
        vulkan_data->parent->imgui_end(*vulkan_data);
    }
}

static void check_vk_result(VkResult err) {
    if (err == 0) return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0) abort();
}

void UIFramework::initGL() { ImGui_ImplOpenGL3_Init(nullptr); }
void UIFramework::initVulkan() {
    vulkan_data = render->getVulkanRenderData();
    auto& inst  = vulkan_data->parent->instance;
    auto& dev   = vulkan_data->parent->device;

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance                  = inst.instance;
    init_info.PhysicalDevice            = dev.physical_device.physical_device;
    init_info.Device                    = dev.device;
    init_info.QueueFamily               = vulkan_data->graphics_queue_family;
    init_info.Queue                     = vulkan_data->graphics_queue;
    init_info.DescriptorPool            = vulkan_data->parent->descriptor_pool;
    init_info.Allocator                 = inst.allocation_callbacks;
    init_info.MinImageCount             = 3;
    init_info.ImageCount                = vulkan_data->swapchain_images.size();
    init_info.CheckVkResultFn           = check_vk_result;
    ImGui_ImplVulkan_Init(&init_info, vulkan_data->render_pass);
    LoadFonts();
    uploadFontsForVulkan();
}
void UIFramework::initSDL() {}
void UIFramework::initGLFW() {
    GLFWwindow* glfw_window = (GLFWwindow*)(window->getWindow());
    if (render->getType() == IRender::RenderType::OpenGL) ImGui_ImplGlfw_InitForOpenGL(glfw_window, true);
    if (render->getType() == IRender::RenderType::Vulkan) ImGui_ImplGlfw_InitForVulkan(glfw_window, true);
}



void UIFramework::uploadFontsForVulkan() {
    VkResult err;
    // Use any command queue
    VkCommandPool command_pool = vulkan_data->command_pool;
    auto          dev          = vulkan_data->parent->device.device;

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool                 = command_pool;
    allocInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount          = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(dev, &allocInfo, &command_buffer);

    // err                 = vkResetCommandPool(dev, command_pool, 0);
    // check_vk_result(err);
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    err = vkBeginCommandBuffer(command_buffer, &begin_info);
    check_vk_result(err);

    ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

    err = vkEndCommandBuffer(command_buffer);
    check_vk_result(err);

    VkSubmitInfo end_info       = {};
    end_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    end_info.commandBufferCount = 1;
    end_info.pCommandBuffers    = &command_buffer;

    err = vkQueueSubmit(vulkan_data->present_queue, 1, &end_info, VK_NULL_HANDLE);
    check_vk_result(err);

    err = vkDeviceWaitIdle(dev);
    check_vk_result(err);
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

}  // namespace fl