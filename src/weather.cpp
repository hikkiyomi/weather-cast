#include "../include/weather.h"

#include <conio.h>
#include <windows.h>

#include <iostream>

constexpr char kEscapeKey = 0x1B;
constexpr char kKeyN      = 'n';
constexpr char kKeyP      = 'p';
constexpr char kKeyPlus   = '=';
constexpr char kKeyMinus  = '-';

Forecaster::Forecaster()
    : handler_(new Handler())
{
    days_shown_ = handler_->GetForecastDays();
}

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
    days_shown_ = handler_->GetForecastDays();
    redraw_needed = true;
    last_update = std::chrono::system_clock::now();
}

WeatherData* Forecaster::GetData(size_t index) {
    if (!bufferized_data.count(index)) {
        bufferized_data[index] = handler_->Request(index);
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

void IncreaseDays(Forecaster* forecaster) {
    if (forecaster->days_shown_ < kForecastDaysRequest) {
        ++forecaster->days_shown_;
        forecaster->redraw_needed = true;
    }
}

void DecreaseDays(Forecaster* forecaster) {
    if (forecaster->days_shown_ > 1) {
        --forecaster->days_shown_;
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
            case kKeyPlus:
                IncreaseDays(forecaster);
                break;
            case kKeyMinus:
                DecreaseDays(forecaster);
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

std::string ProperFloatToString(float x) {
    std::string s = std::to_string(x);

    while (s[s.size() - 2] != '.') {
        s.pop_back();
    }

    return (x > 0 ? "+" : "") + s;
}

std::string FormCell(const std::map<uint8_t, Weather>& data, uint8_t hour) {
    uint8_t temp_weathercode = 255;

    if (data.at(hour).weathercode.has_value()) {
        temp_weathercode = data.at(hour).weathercode.value();
    }

    std::string result;
    const auto& pic = GetWeathercodePicture(temp_weathercode);    
    const std::string description = GetDescription(temp_weathercode);

    result += std::string(pic[0]) + description + "\n";
    result += std::string(pic[1]);

    if (data.at(hour).temperature.has_value()) {
        float temperature = data.at(hour).temperature.value();
        result += ProperFloatToString(temperature);
    } else {
        result += "?";
    }

    if (data.at(hour).apparent_temperature.has_value()) {
        float apparent_temperature = data.at(hour).apparent_temperature.value();
        result += " (";
        result += ProperFloatToString(apparent_temperature);
        result += ")";
    }

    result += " ^C\n";
    result += std::string(pic[2]);

    if (data.at(hour).wind_direction.has_value()) {
        auto wind_direction = data.at(hour).wind_direction.value();
        result += std::to_string(wind_direction) + "^ | ";
    } else {
        result += "? | ";
    }

    if (data.at(hour).wind_speed.has_value()) {
        auto wind_speed = data.at(hour).wind_speed.value();
        auto temp = ProperFloatToString(wind_speed);

        if (wind_speed > 0) {
            temp = temp.substr(1);
        }

        result += temp;
    } else {
        result += "?";
    }

    result += " km/h\n";
    result += std::string(pic[3]);

    if (data.at(hour).visibility.has_value()) {
        auto visibility = data.at(hour).visibility.value();
        result += std::to_string(visibility / 1000);
    } else {
        result += "?";
    }

    result += " km\n";
    result += std::string(pic[4]);

    if (data.at(hour).precipitation.has_value()) {
        auto precipitation = data.at(hour).precipitation.value();
        auto temp = ProperFloatToString(precipitation);

        if (precipitation > 0) {
            temp = temp.substr(1);
        }

        result += temp;
    } else {
        result += "?";
    }

    result += " mm | ";

    if (data.at(hour).precipitation_probability.has_value()) {
        auto precipitation_prob = data.at(hour).precipitation_probability.value();
        result += std::to_string(precipitation_prob);
    } else {
        result += "?";
    }
    
    result += " %\n";

    return result;
}

void DrawWeather(const std::string& name, const std::map<uint8_t, Weather>& data) {
    ft_table_t* table = ft_create_table();
    ft_set_border_style(table, FT_BOLD2_STYLE);

    ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
    ft_set_cell_prop(table, FT_ANY_ROW, FT_ANY_COLUMN, FT_CPROP_TEXT_ALIGN, FT_ALIGNED_CENTER);
    ft_set_cell_prop(table, FT_ANY_ROW, FT_ANY_COLUMN, FT_CPROP_MIN_WIDTH, 35);

    const std::string morning_data = FormCell(data, 6);
    const std::string day_data = FormCell(data, 12);
    const std::string evening_data = FormCell(data, 18);
    const std::string night_data = FormCell(data, 0);

    ft_write_ln(table, name.c_str());
    ft_write_ln(table, "Morning", "Day", "Evening", "Night");
    ft_write_ln(table, morning_data.c_str(), day_data.c_str(), evening_data.c_str(), night_data.c_str());

    ft_set_cell_span(table, 0, 0, 4);
    printf("%s\n", ft_to_string(table));

    ft_destroy_table(table);
}

void Draw(Forecaster* forecaster) {
    while (true) {
        if (!RedrawNeeded(forecaster)) {
            continue;
        }

        system("cls");

        WeatherData& api_response = *forecaster->GetData(forecaster->city_shown_);
        auto time_now = std::chrono::system_clock::now();
        auto current_hour = std::chrono::duration_cast<std::chrono::hours>(time_now.time_since_epoch()).count() % 24 + 3;
        auto current_minute = std::chrono::duration_cast<std::chrono::minutes>(time_now.time_since_epoch()).count() % 60;

        std::cout << "Weather forecast: ";
        forecaster->handler_->PrintCity(std::cout, forecaster->city_shown_);
        std::cout << "\n\n\n";

        std::cout << "Time now: " << current_hour << ":" << current_minute << " (GMT+3)\n";
        std::cout << FormCell(api_response.begin()->second, current_hour) << "\n\n";

        size_t drawn = 0;

        for (const auto& [day, data] : api_response) {
            DrawWeather(day, data);
            ++drawn;

            if (drawn >= forecaster->days_shown_) {
                break;
            }
        }

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
