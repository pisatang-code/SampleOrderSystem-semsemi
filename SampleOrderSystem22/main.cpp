#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <memory>
#include "Model/SampleStorage.h"
#include "Model/OrderStorage.h"
#include "View/ConsoleView.h"
#include "Controller/MainController.h"

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    auto sampleStorage = std::make_shared<SampleStorage>();
    auto orderStorage  = std::make_shared<OrderStorage>();
    auto view          = std::make_shared<ConsoleView>();

    MainController controller(sampleStorage, orderStorage, view);
    controller.run();

    return 0;
}
