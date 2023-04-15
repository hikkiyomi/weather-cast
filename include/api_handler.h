#pragma once

#include "parser.h"

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include <any>
#include <cinttypes>
#include <filesystem>
#include <iostream>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

constexpr float    kUndefinedLatitude    = -100;
constexpr float    kUndefinedLongitude   = -100;
const std::string& kUndefinedCity        = "UndefinedCity";
const std::string& kUndefinedCountry     = "UndefinedCountry";
constexpr uint16_t kDefaultForecastDays  = 4;
constexpr uint16_t kForecastDaysRequest  = 16;
constexpr uint32_t kDoNotUpdateFrequency = 0;
constexpr size_t   kDefaultCity          = 123456;

struct Weather {
    std::optional<float> temperature;
    std::optional<float> apparent_temperature;
    std::optional<uint16_t> precipitation_probability;
    std::optional<float> precipitation;
    std::optional<uint8_t> weathercode;
    std::optional<uint16_t> visibility;
    std::optional<float> wind_speed;
    std::optional<uint16_t> wind_direction;
};

using WeatherData = std::map<std::string, std::map<uint8_t, Weather>>;
using LoadsType = nlohmann::json_abi_v3_11_2::basic_json<>;

struct City {
    std::string name;
    std::string country;

    City(const std::string& _name, const std::string& _country);

    friend std::ostream& operator<<(std::ostream& stream, const City& name);
};

struct Settings {
    float latitude           = kUndefinedLatitude;
    float longitude          = kUndefinedLongitude;
    std::vector<City> cities = {};
    uint16_t forecast_days   = kDefaultForecastDays;
    uint32_t update_freq     = kDoNotUpdateFrequency;

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

    [[nodiscard]] inline const std::vector<City>& GetCities() const noexcept {
        return settings_.cities;
    }

    [[nodiscard]] inline const City& GetCity(size_t index) const noexcept {
        return settings_.cities[index];
    }

    inline ConfigParser& SetCityCountry(size_t index, std::string_view country) noexcept {
        settings_.cities[index].country = country;
        return *this;
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
    WeatherData* Request(size_t city_index);
    bool HasNext(size_t city_index);
    bool HasPrev(size_t city_index);
    void PrintCity(std::ostream& stream, size_t city_index);
public:
    uint32_t GetFrequency() const;
    uint16_t GetForecastDays() const;
private:
    std::filesystem::path config_path_;
    ConfigParser config_;
};

template<typename T>
class WeatherValue {
public:
    std::optional<T> operator()(
        const LoadsType& loads,
        std::string_view variable,
        size_t index
    ) {
        auto temp = loads["hourly"][variable][index];

        if (temp.is_null()) {
            return {};
        }

        return temp;
    }
};
