#include "fl/stdafx.hpp"
#include <SDL_vulkan.h>

namespace fl {

extern void* create_surface_sdl2 (void* instance, void* window) {
	VkSurfaceKHR surface = nullptr;
	if (!SDL_Vulkan_CreateSurface((SDL_Window*)window, (VkInstance)instance, &surface))
    {
        throw std::runtime_error("Unable to create Vulkan compatible surface using SDL");
    }
	return (void*)surface;
}

}