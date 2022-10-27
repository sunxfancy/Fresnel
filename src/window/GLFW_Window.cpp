#include "fl/window/GLFW_Window.hpp"
#include "fl/render/IRender.hpp"

#include <GLFW/glfw3.h>

namespace fl {

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

GLFW_Window::GLFW_Window() {}

GLFW_Window::~GLFW_Window() {}

void GLFW_Window::Init() {
    createWindow();
}

void GLFW_Window::createWindow() {
    // glfw: initialize and configure
    // ------------------------------
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) throw std::runtime_error("Failed to initialize GLFW\n");

    glfwWindowHint(GLFW_RESIZABLE, config->resizable ? GLFW_TRUE:GLFW_FALSE);

    if (config->enable_opengl_support) {
        enable_gl = true;
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, config->OpenGL_Version_Major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, config->OpenGL_Version_Minor);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);  // uncomment this statement to fix compilation on OS X
#endif
    }

    if (config->enable_vulkan_support) {
        enable_vulkan = true;
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }

    // glfw window creation
    // --------------------
    window = glfwCreateWindow(config->width, config->height, config->window_title.c_str(), NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window\n");
    }

    if (config->enable_opengl_support) {
        glfwMakeContextCurrent(window);
        glfwSwapInterval(config->enable_vsync ? 1 : 0);  // Enable vsync
    }
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
}

void GLFW_Window::mainLoop() {
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // poll IO events and handle them (keys pressed/released, mouse moved etc.)
        processInput();

        if (render_cb) render_cb->onRender();

        // glfw: swap buffers
        swapBuffers();
    }
    cleanUp();
}

void GLFW_Window::swapBuffers() {
    if (enable_gl) glfwSwapBuffers(window);
    if (enable_vulkan && render) render->present();
}

void GLFW_Window::cleanUp() {
    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwDestroyWindow(window);
    glfwTerminate();
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void GLFW_Window::processInput() {
    glfwPollEvents();
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
}
void GLFW_Window::setPresentCallback(IRender* render) { this->render = render; }
void GLFW_Window::setRenderCallback(IWindowRenderCallback* callback) { render_cb = callback; }
void GLFW_Window::setKeyboardCallback(IWindowKeyboardCallback* callback) { keyboard_cb = callback; }
void GLFW_Window::setMouseCallback(IWindowMouseCallback* callback) { mouse_cb = callback; }


extern void* create_surface_glfw (void* instance, void* window);

void*        GLFW_Window::getWindow() { return window; }
GLLoader     GLFW_Window::getGLLoader() { return (GLLoader)glfwGetProcAddress; }
VulkanLoader GLFW_Window::getVulkanLoader() { return create_surface_glfw; }

}  // namespace fl