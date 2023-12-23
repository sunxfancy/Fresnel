
#include "fl/system/App.hpp"
#include <clocale>

namespace fl
{
    

App::App() {

}

App::~App() {

}

void App::Init() {
    std::setlocale(LC_ALL, "en_US.utf8");
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
