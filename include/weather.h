#pragma once

#include "api_handler.h"
#include "weathercodes.h"

#include "fort.hpp"

#include <chrono>

constexpr size_t kStartingCity = 0;

class Forecaster {
public:
    Forecaster();

    Forecaster(const Forecaster& other) = delete;
    Forecaster& operator=(const Forecaster& other) = delete;
    ~Forecaster();
public:
    void ChangeConfig(const std::filesystem::path& config_path);
    void Run();
    WeatherData* GetData(size_t index);
public:
    friend void HandleKeyboard(Forecaster* forecaster);
    friend void Draw(Forecaster* forecaster);
    friend void IncreaseIndex(Forecaster* forecaster);
    friend void DecreaseIndex(Forecaster* forecaster);
    friend void IncreaseDays(Forecaster* forecaster);
    friend void DecreaseDays(Forecaster* forecaster);
    friend bool RedrawNeeded(Forecaster* forecaster);
    friend bool UpdateNeeded(Forecaster* forecaster);
private:
    Handler* handler_;
    size_t city_shown_ = kStartingCity;
    uint16_t days_shown_;
    bool redraw_needed = true;
    std::chrono::time_point<std::chrono::system_clock> last_update;

    std::map<size_t, WeatherData*> bufferized_data;
};
