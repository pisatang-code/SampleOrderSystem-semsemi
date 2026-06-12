#pragma once
#include <string>

class IView {
public:
    virtual ~IView() = default;
    virtual void render() = 0;
    virtual void showMessage(const std::string& msg) = 0;
    virtual int  getInput() = 0;
};
