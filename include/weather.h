#pragma once

#include "api_handler.h"

class Forecaster {
public:
    Forecaster();

    Forecaster(const Forecaster& other) = delete;
    Forecaster& operator=(const Forecaster& other) = delete;
    ~Forecaster();
public:
    void ChangeConfig(const std::filesystem::path& config_path);
    void Run();
private:
    Handler* handler;
};
