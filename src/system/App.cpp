
#include "fl/system/App.hpp"

namespace fl
{
    

App::App() {

}

App::~App() {

}

void App::Init() {
    window->setRenderCallback(this);
    ui->setUICallback(this);
}
 
void App::onRender() {
    render->clear();
    ui->renderUI();
}

void App::onRenderUI() {
}

void App::startApp() {
    window->mainLoop();
}

} // namespace fl
