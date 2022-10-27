#include "fl/window/SDL2_Window.hpp"
#include <SDL.h>

namespace fl {


SDL2_Window::SDL2_Window() {}

SDL2_Window::~SDL2_Window() {}

void SDL2_Window::Init() {
    createWindow();
}

uint32_t SDL2_Window::createFlags() {
    uint32_t flags = 0;
    if (config->full_screen) flags |= SDL_WINDOW_FULLSCREEN;
    if (config->hidden) flags |= SDL_WINDOW_HIDDEN;
    else flags |= SDL_WINDOW_SHOWN;
    if (config->allow_high_DPI) flags |= SDL_WINDOW_ALLOW_HIGHDPI;
    if (config->borderless) flags |= SDL_WINDOW_BORDERLESS;
    if (config->minimized) flags |= SDL_WINDOW_MINIMIZED;
    if (config->maximized) flags |= SDL_WINDOW_MAXIMIZED;
    if (config->resizable) flags |= SDL_WINDOW_RESIZABLE;
    if (config->mouse_capture) flags |= SDL_WINDOW_MOUSE_CAPTURE;
    if (config->always_on_top) flags |= SDL_WINDOW_ALWAYS_ON_TOP;
    if (config->enable_vulkan_support) flags |= SDL_WINDOW_VULKAN;
    if (config->enable_opengl_support) flags |= SDL_WINDOW_OPENGL;
    return flags;
}

void SDL2_Window::createWindow() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER)) {
        string err_msg = "Failed to create SDL window\n";
        throw std::runtime_error(err_msg + SDL_GetError());
    }

    window = SDL_CreateWindow(
        config->window_title.c_str(), 
        config->x == fl::autocenter ? SDL_WINDOWPOS_CENTERED : config->x,
        config->y == fl::autocenter ? SDL_WINDOWPOS_CENTERED : config->y,
        config->width, config->height,
        createFlags());

}

void SDL2_Window::mainLoop() {
    // render loop
    // -----------
    
    while (!done) {
        // input
        processInput();
        if (render_cb) render_cb->onRender();

        // swap buffers 
    }
    cleanUp();
}

void SDL2_Window::cleanUp() {
    // glfw: terminate, clearing all previously allocated GLFW resources.
    SDL_DestroyWindow(window);
    SDL_Quit();
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void SDL2_Window::processInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_WINDOWEVENT &&
            event.window.event == SDL_WINDOWEVENT_CLOSE &&
            event.window.windowID == SDL_GetWindowID(window))
            done = true;
    }
}

void SDL2_Window::setPresentCallback(IRender* render) { this->render = render; }
void SDL2_Window::setRenderCallback(IWindowRenderCallback* callback) { render_cb = callback; }
void SDL2_Window::setKeyboardCallback(IWindowKeyboardCallback* callback) { keyboard_cb = callback; }
void SDL2_Window::setMouseCallback(IWindowMouseCallback* callback) { mouse_cb = callback; }

extern void* create_surface_sdl2 (void* instance, void* window);

void*        SDL2_Window::getWindow() { return window; }
GLLoader     SDL2_Window::getGLLoader() { return SDL_GL_GetProcAddress; }
VulkanLoader SDL2_Window::getVulkanLoader() { return create_surface_sdl2; }

}  // namespace fl