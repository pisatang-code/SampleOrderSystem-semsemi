#pragma once
#include "IController.h"
#include "SampleController.h"
#include "OrderController.h"
#include "ProductionController.h"
#include "MonitoringController.h"
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
    std::shared_ptr<SampleStorage>        m_sampleStorage;
    std::shared_ptr<OrderStorage>         m_orderStorage;
    std::shared_ptr<ConsoleView>          m_view;
    std::shared_ptr<SampleController>     m_sampleCtrl;
    std::shared_ptr<OrderController>      m_orderCtrl;
    std::shared_ptr<ProductionController> m_productionCtrl;
    std::shared_ptr<MonitoringController> m_monitoringCtrl;

    ConsoleView::Summary buildSummary() const;

    void handleSampleManagement();   // 메뉴 1
    void handleOrderReception();     // 메뉴 2
    void handleOrderApproval();      // 메뉴 3
    void handleMonitoring();         // 메뉴 4
    void handleProductionLine();     // 메뉴 5
    void handleShipment();           // 메뉴 6
};
