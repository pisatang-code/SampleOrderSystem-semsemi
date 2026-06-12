#pragma once
#include "IController.h"
#include "../View/ConsoleView.h"
#include "../Model/SampleStorage.h"
#include "../Model/OrderStorage.h"
#include <memory>

class MainController : public IController {
public:
    MainController(std::shared_ptr<SampleStorage> sampleStorage,
                   std::shared_ptr<OrderStorage>  orderStorage,
                   std::shared_ptr<ConsoleView>   view);
    void run() override;

private:
    std::shared_ptr<SampleStorage> m_sampleStorage;
    std::shared_ptr<OrderStorage>  m_orderStorage;
    std::shared_ptr<ConsoleView>   m_view;

    ConsoleView::Summary buildSummary() const;
    void handleMenu(int choice);
};
