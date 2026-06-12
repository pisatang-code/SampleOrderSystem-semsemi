#pragma once
#include "IView.h"

class ConsoleView : public IView {
public:
    struct Summary {
        int sampleCount = 0;
        int totalStock  = 0;
        int orderCount  = 0;
        int queueCount  = 0;
    };

    void setSummary(const Summary& summary);
    void render()                              override;
    void showMessage(const std::string& msg)   override;
    int  getInput()                            override;

private:
    Summary m_summary;
};
