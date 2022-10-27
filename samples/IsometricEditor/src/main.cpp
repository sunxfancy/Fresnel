
#include "fl/render/VulkanRender.hpp"
#include "fl/render/VulkanImpl.hpp"
#include "fl/system/App.hpp"
#include "fl/system/Config.hpp"
#include "fl/window/GLFW_Window.hpp"
#include "imgui.h"
#include <filesystem>

using namespace fl;

static DI di;

class MyApp : public App {
public:
    virtual void onRenderUI() {
        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code
        // to learn more about Dear ImGui!).
        if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            ImGui::Begin(u8"你好");  // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");           // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);  // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);  // Edit 1 float using a slider from 0.0f to 1.0f
            ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
            ImGui::ColorEdit3("clear color", (float*)&clear_color);  // Edit 3 floats representing a color

            if (ImGui::Button(
                    "Button"))  // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                        ImGui::GetIO().Framerate);
            ImGui::End();
        }

        {
            ImGui::Begin(u8"渲染界面");
            sptr<IRender> render = di.get<IRender>("IRender");
            RenderData* rd = render->getVulkanRenderData();
            ImGui::Image((ImTextureID)(intptr_t) rd->getCurrentImage(),ImVec2(600, 400));
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window) {
            ImGui::Begin("Another Window",
                         &show_another_window);  // Pass a pointer to our bool variable (the window will have a closing
                                                 // button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me")) show_another_window = false;
            ImGui::End();
        }
    }

    

protected:
    bool  show_demo_window = true;
    bool  show_another_window = false;
    float f       = 0.0f;
    int   counter = 0;
};

void configDI(DI& di) {
    di["IWindow"]     = []() -> IModule* { return new GLFW_Window(); };
    di["IRender"]     = []() -> IModule* { return new VulkanRender(); };
    di["Config"]      = []() -> IModule* { auto p = new Config(); p->enable_vulkan_support = true; return p; };
    di["UIFramework"] = []() -> IModule* { return new UIFramework(); };
    di["App"]         = []() -> IModule* { return new MyApp(); };
}

static std::filesystem::path getExecutablePath(char* argv0) {
    std::filesystem::path argv0path(argv0);
    if (argv0path.is_absolute()) return argv0path;
    std::filesystem::path curr = std::filesystem::current_path() / argv0path;
    return curr.native();
} 



int main(int argc, char* argv[]) {
    std::cout << getExecutablePath(argv[0]) << '\n';

    configDI(di);
    sptr<App> app = di.get<App>("App");
    app->startApp();
    return 0;
}