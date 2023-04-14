#include "../include/weather.h"

#include <conio.h>
#include <windows.h>

#include <iostream>

constexpr char kEscapeKey = 0x1B;
constexpr char kKeyN = 'n';
constexpr char kKeyP = 'p';

Forecaster::Forecaster()
    : handler_(new Handler())
{}

Forecaster::~Forecaster() {
    delete handler_;

    for (auto [index, data] : bufferized_data) {
        delete data;
    }
}

void Forecaster::ChangeConfig(const std::filesystem::path& config_path) {
    delete handler_;
    
    for (auto [index, data] : bufferized_data) {
        delete data;
    }

    handler_ = new Handler(config_path);
    city_shown_ = kStartingCity;
    redraw_needed = true;
    last_update = std::chrono::system_clock::now();
}

WeatherData* Forecaster::GetData(size_t index) {
    if (!bufferized_data.count(index)) {
        bufferized_data[index] = handler_->Request(index);
    } else {

    }

    return bufferized_data[index];
}

void IncreaseIndex(Forecaster* forecaster) {
    if (forecaster->handler_->HasNext(forecaster->city_shown_)) {
        ++forecaster->city_shown_;
        forecaster->redraw_needed = true;
    }
}

void DecreaseIndex(Forecaster* forecaster) {
    if (forecaster->handler_->HasPrev(forecaster->city_shown_)) {
        --forecaster->city_shown_;
        forecaster->redraw_needed = true;
    }
}

void HandleKeyboard(Forecaster* forecaster) {
    while (true) {
        char key_pressed = getch();

        switch (key_pressed) {
            case kEscapeKey:
                system("cls");
                exit(0);
            case kKeyN:
                IncreaseIndex(forecaster);
                break;
            case kKeyP:
                DecreaseIndex(forecaster);
                break;
        }
    }
}

bool UpdateNeeded(Forecaster* forecaster) {
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    uint32_t freq = forecaster->handler_->GetFrequency();
    bool update_needed = (freq != kDoNotUpdateFrequency && std::chrono::duration_cast<std::chrono::seconds>(now - forecaster->last_update).count() >= freq);

    if (update_needed) {
        for (auto [index, data] : forecaster->bufferized_data) {
            delete data;
        }

        forecaster->bufferized_data.clear();
    }

    return update_needed;
}

bool RedrawNeeded(Forecaster* forecaster) {
    return forecaster->redraw_needed || UpdateNeeded(forecaster);
}

void Draw(Forecaster* forecaster) {
    while (true) {
        if (!RedrawNeeded(forecaster)) {
            continue;
        }

        system("cls");
        forecaster->GetData(forecaster->city_shown_);

        std::cout << "Weather forecast: ";
        forecaster->handler_->PrintCity(std::cout, forecaster->city_shown_);
        std::cout << "\n";

        forecaster->redraw_needed = false;
        forecaster->last_update = std::chrono::system_clock::now();
    }
}

void Forecaster::Run() {
    BlockInput(true);

    std::vector<std::thread> threads;

    threads.push_back(std::thread(HandleKeyboard, this));
    threads.push_back(std::thread(Draw, this));
    
    for (auto& thread : threads) {
        thread.join();
    }
}
