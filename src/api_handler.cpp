#include "../include/api_handler.h"

#include <iostream>
#include <utility>

constexpr uint16_t kStatusOK = 200;

std::map<size_t, std::map<std::string, std::any>> buffered_city_info;

City::City(const std::string& _name, const std::string& _country)
    : name(_name)
    , country(_country)
{}

std::ostream& operator<<(std::ostream& stream, const City& city) {
    stream << city.name << ", " << city.country;
    return stream;
}

ConfigParser::ConfigParser(const std::filesystem::path& config_path)
    : parser_(omfl::parse(config_path))
{
    ParseConfig();
}

[[nodiscard]] bool CheckIfUndefinedPlace(const Settings& settings) noexcept {
    return settings.latitude == kUndefinedLatitude
        && settings.longitude == kUndefinedLongitude
        && settings.cities.empty();
}

void ConfigParser::ParseConfig() {
    try {
        settings_.latitude = parser_.Get("latitude").AsFloat();
    } catch (const std::runtime_error&) {}
    
    try {
        settings_.longitude = parser_.Get("longitude").AsFloat();
    } catch (const std::runtime_error&) {}

    try {
        auto cities_array = parser_.Get("city");

        for (size_t index = 0;; ++index) {
            auto temp = cities_array[index];

            if (temp.GetType() == omfl::Type::Undefined) {
                break;
            }

            settings_.cities.push_back(
                City(
                    std::string(temp.AsString()),
                    kUndefinedCountry
                )
            );
        }
    } catch (const std::runtime_error&) {}

    try {
        settings_.forecast_days = std::max(1, std::min(16, parser_.Get("days").AsInt()));
    } catch (const std::runtime_error&) {}
    
    try {
        settings_.update_freq = parser_.Get("frequency").AsInt();
    } catch (const std::runtime_error&) {}

    if (CheckIfUndefinedPlace(settings_)) {
        throw std::runtime_error("No information about city is provided.");
    }
}

Handler::Handler(const std::filesystem::path& config_path)
    : config_path_(config_path)
    , config_(ConfigParser(config_path))
{}

[[nodiscard]] bool RestoreNeeded(const ConfigParser& config, size_t city_index) noexcept {
    return config.GetLatitude() == kUndefinedLatitude
        || config.GetLongitude() == kUndefinedLongitude
        || config.GetCities().size() > 1
        || config.GetCity(city_index).country == kUndefinedCountry;
}

void RestoreCoordinatesBufferized(ConfigParser& config, size_t city_index) {
    config
        .SetLatitude(std::any_cast<float>(buffered_city_info[city_index]["latitude"]))
        .SetLongitude(std::any_cast<float>(buffered_city_info[city_index]["longitude"]));
}

void RestoreCoordinates(ConfigParser& config, size_t city_index) {
    const static std::string api_key = "lQTzZriyKM8cFys/Sa20ug==feHpFj2EgiUdz8Ps";
    const static auto url = cpr::Url{"https://api.api-ninjas.com/v1/city"};
    const static auto header = cpr::Header{
        {"User-Agent", "BOT"},
        {"X-Api-Key", api_key}
    };

    assert(city_index < config.GetCities().size());

    if (buffered_city_info.count(city_index)) {
        RestoreCoordinatesBufferized(config, city_index);
        return;
    }

    auto params = cpr::Parameters{{"name", config.GetCity(city_index).name}};
    auto response = cpr::Get(url, params, header);

    while (response.status_code != kStatusOK) {
        response = cpr::Get(url, params, header);
    }
    
    auto loads = nlohmann::json::parse(response.text);

    if (loads.size() == 0) {
        throw std::runtime_error("You might have provided a non-existing city. Please, fix.");
    }

    config
        .SetLatitude(loads[0]["latitude"])
        .SetLongitude(loads[0]["longitude"])
        .SetCityCountry(city_index, std::string(loads[0]["country"]));

    buffered_city_info[city_index]["latitude"] = loads[0]["latitude"].get<float>();
    buffered_city_info[city_index]["longitude"] = loads[0]["longitude"].get<float>();
}

void FormParameters(const ConfigParser& config, cpr::Parameters& params) {
    params.Add(cpr::Parameter{"latitude", std::to_string(config.GetLatitude())});
    params.Add(cpr::Parameter{"longitude", std::to_string(config.GetLongitude())});
    params.Add(cpr::Parameter{"forecast_days", std::to_string(kForecastDaysRequest)});
    
    std::string hourly_info;
    const auto& full_info = config.GetWeatherVariables();

    for (const auto& weather_info : full_info) {
        hourly_info += weather_info;
        hourly_info.push_back(',');
    }

    hourly_info.pop_back();

    params.Add(cpr::Parameter{"hourly", hourly_info});
    params.Add(cpr::Parameter{"timezone", "Europe/Moscow"});
}

std::pair<std::string, uint8_t> ParseDate(const std::string& datetime) {
    std::pair<std::string, uint8_t> result = {"", 0};
    
    for (size_t i = 0; i < datetime.size(); ++i) {
        if (datetime[i] == 'T') {
            assert(i + 2 < datetime.size());
            result.second = (datetime[i + 1] - '0') * 10 + (datetime[i + 2] - '0');

            break;
        } else {
            result.first.push_back(datetime[i]);
        }
    }

    return result;
}

WeatherData* ParseLoads(const LoadsType& loads) {
    auto* weather = new WeatherData;
    size_t data_amount = loads["hourly"]["time"].size();
    
    WeatherValue<float> float_value;
    WeatherValue<uint8_t> uint8_value;
    WeatherValue<uint16_t> uint16_value;

    for (size_t i = 0; i < data_amount; ++i) {
        auto [date, time] = ParseDate(loads["hourly"]["time"][i]);
        
        (*weather)[date][time] = Weather{
            float_value(loads, "temperature_2m", i),
            float_value(loads, "apparent_temperature", i),
            uint16_value(loads, "precipitation_probability", i),
            float_value(loads, "precipitation", i),
            uint8_value(loads, "weathercode", i),
            uint16_value(loads, "visibility", i),
            float_value(loads, "windspeed_10m", i),
            uint16_value(loads, "winddirection_10m", i)
        };
    }

    return weather;
}

WeatherData* Handler::Request(size_t city_index) {
    if (RestoreNeeded(config_, city_index)) {
        RestoreCoordinates(config_, city_index);
    }

    const static auto url = cpr::Url{"https://api.open-meteo.com/v1/forecast"};
    const static auto header = cpr::Header{{"User-Agent", "BOT"}};
    auto params = cpr::Parameters{};

    FormParameters(config_, params);

    auto request = cpr::Get(url, params, header);

    while (request.status_code != kStatusOK) {
        request = cpr::Get(url, params, header);
    }

    auto loads = nlohmann::json::parse(request.text);

    if (!loads["error"].is_null()) {
        assert(loads["error"]);
        throw std::runtime_error("You might have provided incorrect weather variables. Please, check your config.");
    }

    return ParseLoads(loads);
}

bool Handler::HasNext(size_t city_index) {
    return city_index < config_.GetCities().size() - 1;
}

bool Handler::HasPrev(size_t city_index) {
    return city_index >= 1;
}

void Handler::PrintCity(std::ostream& stream, size_t city_index) {
    stream << config_.GetCity(city_index);
}

uint32_t Handler::GetFrequency() const {
    return config_.GetUpdateFrequency();
}

uint16_t Handler::GetForecastDays() const {
    return config_.GetForecastDays();
}
