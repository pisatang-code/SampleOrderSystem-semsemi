#pragma once

class IController {
public:
    virtual ~IController() = default;
    virtual void run() = 0;
};
