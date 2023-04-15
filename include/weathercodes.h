#pragma once

#include <string>

const std::vector<std::string_view> kCodeUnknown = {
            "    .-.      ",
			"     __)     ",
			"    (        ",
			"     `-'     ",
			"      •      "
};

const std::vector<std::string_view> kCodeCloudy = {
            "             ",
			"     .--.    ",
			"  .-(    ).  ", 
			" (___.__)__) ", 
			"             "
};

const std::vector<std::string_view> kCodeFog = {
            "             ",
			" _ - _ - _ - ",
			"  _ - _ - _  ",
			" _ - _ - _ - ",
			"             "
};

const std::vector<std::string_view> kCodeHeavyRain = {
            "     .-.     ",
			"    (   ).   ",
			"   (___(__)  ",
			"  ,',',','   ",
			"  ,',',','   "
};

const std::vector<std::string_view> kCodeHeavyShowers = {
            " _`/\"\".-.    ",
			"  ,\\_(   ).  ",
			"   /(___(__) ",
			"   ,',',','  ",
			"   ,',',','  "
};

const std::vector<std::string_view> kCodeHeavySnow = {
            "     .-.     ",
			"    (   ).   ",
			"   (___(__)  ",
			"   * * * *   ",
			"  * * * *    "
};

const std::vector<std::string_view> kCodeHeavySnowShowers = {
            " _`/\"\".-.    ",
			"  ,\\_(   ).  ",
			"   /(___(__) ",
			"    * * * *  ",
			"   * * * *   "
};

const std::vector<std::string_view> kCodeLightRain = {
            "     .-.     ",
			"    (   ).   ",
			"   (___(__)  ",
			"    ' ' ' '  ",
			"   ' ' ' '   "
};

const std::vector<std::string_view> kCodeLightShowers = {
            " _`/\"\".-.    ",
			"  ,\\_(   ).  ",
			"   /(___(__) ",
			"     ' ' ' ' ",
			"    ' ' ' '  "
};

const std::vector<std::string_view> kCodeLightSleet = {
            "     .-.     ",
			"    (   ).   ",
			"   (___(__)  ",
			"    ' * ' *  ",
			"   * ' * '   "
};

const std::vector<std::string_view> kCodeLightSleetShowers = {
            " _`/\"\".-.    ",
			"  ,\\_(   ).  ",
			"   /(___(__) ",
			"     ' * ' * ",
			"    * ' * '  "
};

const std::vector<std::string_view> kCodeLightSnow = {
            "     .-.     ",
			"    (   ).   ",
			"   (___(__)  ",
			"    *  *  *  ",
			"   *  *  *   "
};

const std::vector<std::string_view> kCodeLightSnowShowers = {
            " _`/\"\".-.    ",
			"  ,\\_(   ).  ",
			"   /(___(__) ",
			"     *  *  * ",
			"    *  *  *  "
};

const std::vector<std::string_view> kCodePartlyCloudy = {
            "   \\         ",
			" _ /\"\".-.    ",
			"   \\_(   ).  ",
			"   /(___(__) ",
			"             "
};

const std::vector<std::string_view> kCodeSunny = {
            "    \\   /    ",
			"     .-.     ",
			"  - (   ) -  ",
			"     `-'     ",
			"    /   \\    "
};

const std::vector<std::string_view> kCodeThunderyHeavyRain = {
            "     .-.     ",
			"    (   ).   ",
			"   (___(__)  ",
			"  ,'7',7,'   ",
			"  ,','7','   "
};

const std::vector<std::string_view> kCodeThunderyShowers = {
            " _`/\"\".-.    ",
			"  ,\\_(   ).  ",
			"   /(___(__) ",
			"    7' '7' ' ",
			"    ' ' ' '  "
};

const std::vector<std::string_view> kCodeThunderySnowShowers = {
            " _`/\"\".-.    ",
			"  ,\\_(   ).  ",
			"   /(___(__) ",
			"     *7 *7 * ",
			"    *  *  *  "
};

const std::vector<std::string_view> kCodeVeryCloudy = {
            "             ",
			"     .--.    ",
			"  .-(    ).  ",
			" (___.__)__) ",
			"             "
};

inline const std::vector<std::string_view>& GetWeathercodePicture(uint8_t weathercode) {
    switch (weathercode) {
        case 0:
            return kCodeSunny;
        case 1: case 2:
            return kCodePartlyCloudy;
        case 3:
            return kCodeVeryCloudy;
        case 45: case 48:
            return kCodeFog;
        case 51: case 53: case 55: case 56: case 57: case 61:
            return kCodeLightRain;
        case 63: case 65: case 66: case 67:
            return kCodeHeavyRain;
        case 71: case 77:
            return kCodeLightSnow;
        case 73: case 75:
            return kCodeHeavySnow;
        case 80:
            return kCodeLightShowers;
        case 81: case 82:
            return kCodeHeavyShowers;
        case 85:
            return kCodeLightSnowShowers;
        case 86:
            return kCodeHeavySnowShowers;
        case 95:
            return kCodeThunderyHeavyRain;
        case 96: case 99:
            return kCodeThunderySnowShowers;
    }

    return kCodeUnknown;
}

inline std::string GetDescription(uint8_t weathercode) {
    switch (weathercode) {
        case 0:
            return "Clear sky";
        case 1:
            return "Mainly clear";
        case 2:
            return "Partly cloudy";
        case 3:
            return "Overcast";
        case 45:
            return "Fog";
        case 48:
            return "Rime fog";
        case 51: case 53: case 55:
            return "Drizzle";
        case 56: case 57:
            return "Freezing drizzle";
        case 61:
            return "Slight rain";
        case 63:
            return "Rain";
        case 65:
            return "Heavy rain";
        case 66: case 67:
            return "Freezing rain";
        case 71: case 73: case 75:
            return "Snowfall";
        case 77:
            return "Snow grains";
        case 80: case 81: case 82:
            return "Rain showers";
        case 85: case 86:
            return "Snow showers";
        case 95: case 96: case 99:
            return "Thunderstorm";
    }

    return "Unknown";
}
