#include "fl/render/GLRender.hpp"
#include "fl/stdafx.hpp"
#include "fl/system/Config.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>



namespace fl {

GLRender::GLRender() {}

GLRender::~GLRender() {}

void GLRender::Init() {
    initGL();
}

void GLRender::setRenderTree() { printf("set Render Tree!\n"); }

void GLRender::initGL() {
    GLLoader load_func = window->getGLLoader();
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader(load_func)) {
        throw std::runtime_error("Failed to initialize GLAD\n");
    }
}

void GLRender::clear(glm::vec4 color) {
    glViewport(0, 0, display_w, display_h);
    glClearColor(color.x, color.y, color.z, color.w);
    glClear(GL_COLOR_BUFFER_BIT);
}


}  // namespace fl