#pragma once

#include "parser.h"

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <cinttypes>
#include <stdexcept>
#include <string>
#include <vector>

constexpr float            kUndefinedLatitude    = -100;
constexpr float            kUndefinedLongitude   = -100;
constexpr std::string_view kUndefinedCity        = "UndefinedCity";
constexpr uint16_t         kDefaultForecastDays  = 4;
constexpr uint32_t         kDoNotUpdateFrequency = 0;

struct Settings {
    float latitude         = kUndefinedLatitude;
    float longitude        = kUndefinedLongitude;
    std::string_view city  = kUndefinedCity;
    uint16_t forecast_days = kDefaultForecastDays;
    uint32_t update_freq   = kDoNotUpdateFrequency;

    std::vector<std::string> weather_variables = {
        "temperature_2m", "apparent_temperature",
        "precipitation_probability", "precipitation",
        "weathercode", "visibility",
        "windspeed_10m", "winddirection_10m"
    };
};

class ConfigParser {
public:
    ConfigParser(const std::filesystem::path& config_path);
public:
    [[nodiscard]] inline const Settings& GetSettings() const noexcept {
        return settings_;
    }

    [[nodiscard]] inline float GetLatitude() const noexcept {
        return settings_.latitude;
    }

    inline ConfigParser& SetLatitude(float latitude) noexcept {
        settings_.latitude = latitude;
        return *this;
    }

    [[nodiscard]] inline float GetLongitude() const noexcept {
        return settings_.longitude;
    }

    inline ConfigParser& SetLongitude(float longitude) noexcept {
        settings_.longitude = longitude;
        return *this;
    }

    [[nodiscard]] inline std::string_view GetCity() const noexcept {
        return settings_.city;
    }

    [[nodiscard]] inline uint16_t GetForecastDays() const noexcept {
        return settings_.forecast_days;
    }

    [[nodiscard]] inline uint32_t GetUpdateFrequency() const noexcept {
        return settings_.update_freq;
    }

    [[nodiscard]] inline const std::vector<std::string>& GetWeatherVariables() const noexcept {
        return settings_.weather_variables;
    }
private:
    Settings settings_;
    omfl::Parser parser_;
private:
    void ParseConfig();
};

class Handler {
public:
    Handler(const std::filesystem::path& config_path = "config.omfl");

    Handler(const Handler& other) = delete;
    Handler& operator=(const Handler& other) = delete;
    ~Handler() = default;
public:
    void Request();
private:
    std::filesystem::path config_path_;
    ConfigParser config_;
};
