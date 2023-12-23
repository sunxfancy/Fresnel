#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "fl/stdafx.hpp"

namespace fl {

extern void* create_vulkan_surface_glfw (void* instance, void* window) {
	VkSurfaceKHR surface = nullptr;
	VkResult err = glfwCreateWindowSurface ((VkInstance)instance, (GLFWwindow*)window, NULL, &surface);
	if (err) {
		const char* error_msg;
		int ret = glfwGetError (&error_msg);
		throw std::runtime_error(error_msg);
	}
	return (void*)surface;
}

extern void* create_gl_surface_glfw(void* window) {
	return nullptr;
}


}