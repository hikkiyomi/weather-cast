#include "../include/parser.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <stack>
#include <stdexcept>

std::vector<std::string_view> ParseWay(std::string_view str);
bool CheckKeyValidity(std::string_view key);
omfl::Type GetValueType(std::string_view value);
std::pair<std::any, bool> ConvertValue(const std::string& value, const omfl::Type& type);
void PrettifyString(std::string& str);
std::pair<std::vector<std::string>, bool> ParseSections(std::string_view str, size_t& index);
std::pair<std::any, bool> ConstructValueArray(std::string_view value);
bool Update(omfl::Parser& parser, const std::vector<std::string>& current_sections, std::string& current_key, std::string& current_value);

omfl::Item::Item(std::string_view _key, const std::any& _value, Type _value_type)
    : key(_key)
    , value(_value)
    , value_type(_value_type)
{}

const std::string& omfl::Item::GetKey() const {
    return key;
}

std::any& omfl::Item::GetValue() {
    return value;
}

const omfl::Type& omfl::Item::GetType() const {
    return value_type;
}

std::vector<std::string_view> ParseWay(std::string_view str) {
    std::vector<std::string_view> result;
    int last_string_index = 0;
    size_t index = 0;

    for (; index < str.size(); ++index) {
        char c = str[index];

        if (c == '.') {
            result.emplace_back(str.substr(last_string_index, index - last_string_index));
            last_string_index = index + 1;
        }
    }

    if (index - last_string_index + 1 > 0) {
        result.emplace_back(str.substr(last_string_index, index - last_string_index + 1));
    }

    return result;
}

const omfl::Item& omfl::Item::Get(std::string_view name) const {
    if (name.find('.') != std::string::npos) {
        return Get(ParseWay(name), 0);
    }

    if (value_type != Type::Section) {
        return *this;
    }

    const auto& items = std::any_cast<const std::map<std::string, Item, std::less<>>&>(value);

    if (items.find(name) == items.end()) {
        throw std::runtime_error("Addressing to an non-existing key/section.");
    }

    return items.find(name)->second;
}

const omfl::Item& omfl::Item::Get(const std::vector<std::string_view>& way, size_t index) const {
    if (index == way.size()) {
        return *this;
    }

    const auto& items = std::any_cast<const std::map<std::string, Item, std::less<>>&>(value);

    return items.find(way[index])->second.Get(way, index + 1);
}

bool omfl::Item::IsInt() const {
    return value_type == Type::Integer;
}

int32_t omfl::Item::AsInt() const {
    return std::any_cast<int32_t>(value);
}

int32_t omfl::Item::AsIntOrDefault(int32_t val) const {
    if (IsInt()) {
        return AsInt();
    }

    return val;
}

bool omfl::Item::IsFloat() const {
    return value_type == Type::Float;
}

double omfl::Item::AsFloat() const {
    return std::any_cast<double>(value);
}

double omfl::Item::AsFloatOrDefault(double val) const {
    if (IsFloat()) {
        return AsFloat();
    }

    return val;
}

bool omfl::Item::IsString() const {
    return value_type == Type::String;
}

std::string_view omfl::Item::AsString() const {
    return std::any_cast<const std::string&>(value);
}

std::string_view omfl::Item::AsStringOrDefault(std::string_view val) const {
    if (IsString()) {
        return AsString();
    }

    return val;
}

bool omfl::Item::IsBool() const {
    return value_type == Type::Boolean;
}

bool omfl::Item::AsBool() const {
    return std::any_cast<bool>(value);
}

bool omfl::Item::AsBoolOrDefault(bool val) const {
    if (IsBool()) {
        return AsBool();
    }

    return val;
}

bool omfl::Item::IsArray() const {
    return value_type == Type::Array;
}

const omfl::Item& omfl::Item::operator[](size_t index) const {
    if (!IsArray()) {
        throw std::runtime_error("Trying to access non-accessible value.");
    }

    const ValueArray& array = std::any_cast<const ValueArray&>(value);

    return array.Get(index);
}

omfl::ValueArray::ValueArray()
    : trash_item_(Item("", "", Type::Undefined))
{}

void omfl::ValueArray::Add(const std::any& value, Type type) {
    values_.push_back(Item("", value, type));
}

const omfl::Item& omfl::ValueArray::Get(size_t index) const {
    if (index < values_.size()) {
        return values_[index];
    }

    return trash_item_;
}

bool omfl::Parser::valid() const {
    return successful_parse_;
}

void omfl::Parser::MarkUnsuccessful() {
    successful_parse_ = false;
}

bool omfl::Parser::Add(const std::vector<std::string>& section_way, const Item& appending_item) {
    return tree_.AddItem(section_way, appending_item);
}

const omfl::Item& omfl::Parser::Get(std::string_view name) const {
    return tree_.GetItem(name);
}

omfl::Parser::Trie::Trie()
    : root_(Item("", std::map<std::string, Item, std::less<>>(), Type::Section))
{}

bool omfl::Parser::Trie::AddItem(const std::vector<std::string>& section_way, const Item& appending_item) {
    Item* current_node = &root_;
    
    for (const auto& section: section_way) {
        auto& items = std::any_cast<std::map<std::string, Item, std::less<>>&>(current_node->GetValue());

        if (items.find(section) == items.end()) {
            items.insert({section, Item(section, std::map<std::string, Item, std::less<>>(), Type::Section)});
        }

        current_node = &items.at(section);
    }

    auto& items = std::any_cast<std::map<std::string, Item, std::less<>>&>(current_node->GetValue());

    if (items.find(appending_item.GetKey()) != items.end()) {
        return false;
    }

    items.insert({appending_item.GetKey(), appending_item});

    return true;
}

const omfl::Item& omfl::Parser::Trie::GetItem(std::string_view name) const {
    return root_.Get(name);
}

bool CheckKeyValidity(std::string_view key) {
    if (key.empty()) {
        return false;
    }

    for (auto character: key) {
        if (
            !std::isalnum(character) &&
            !(character == '-' || character == '_')
        ) {
            return false;
        }
    }

    return true;
}

omfl::Type GetValueType(std::string_view value) {
    using omfl::Type;
    
    if (value.empty()) {
        return Type::Undefined;
    }

    if (value[0] == '\"' && value.back() == '\"') {
        // Presumably, it is a string.

        if (std::count(value.begin(), value.end(), '\"') == 2) {
            return Type::String;
        } else {
            return Type::Undefined;
        }
    } else if (value[0] == '[' && value.back() == ']') {
        int32_t balance = 0;

        for (auto character: value) {
            if (character == '[') {
                ++balance;
            } else if (character == ']') {
                if (balance == 0) {
                    return Type::Undefined;
                }

                --balance;
            }
        }

        if (balance > 0) {
            return Type::Undefined;
        }

        return Type::Array;
    } else if (value == "true" || value == "false") {
        return Type::Boolean;
    } else {
        if (value[0] == '.') {
            return Type::Undefined;
        }

        if ((value[0] == '+' || value[0] == '-') && value.size() == 1) {
            return Type::Undefined;
        }

        if (
            !std::isdigit(value[0]) &&
            !(value[0] == '+' || value[0] == '-')
        ) {
            return Type::Undefined;
        }

        size_t pluses = 0;
        size_t minuses = 0;
        size_t points = 0;
        size_t point_index = value.size() + 1;

        for (size_t i = 0; i < value.size(); ++i) {
            char character = value[i];

            if (character == '+') {
                ++pluses;
            } else if (character == '-') {
                ++minuses;
            } else if (character == '.') {
                ++points;
                point_index = i;
            } else if (!std::isdigit(character)){
                return Type::Undefined;
            }
        }

        if (pluses + minuses > 1 || points > 1) {
            return Type::Undefined;
        }

        if ((pluses == 1 || minuses == 1) && value[0] != '+' && value[0] != '-') {
            return Type::Undefined;
        }

        if (points > 0) {
            assert(point_index != value.size() + 1);
            
            if (
                (point_index == 1 && !std::isdigit(value[0])) ||
                point_index == value.size() - 1
            ) {
                return Type::Undefined;
            }

            return Type::Float;
        }

        return Type::Integer;
    }

    return Type::Undefined;
}

std::pair<std::any, bool> ConstructValueArray(std::string_view value) {
    using omfl::Type;

    omfl::ValueArray result;
    std::string buff;
    bool ok = true;
    int32_t balance = 0;

    for (size_t i = 1; i < value.size(); ++i) {
        if ((value[i] == ',' && balance == 0) || i == value.size() - 1) {
            if (buff.empty()) {
                continue;
            }

            PrettifyString(buff);

            Type type = GetValueType(buff);

            if (type == Type::Undefined) {
                ok = false;

                break;
            }

            auto [val, successful] = ConvertValue(buff, type);

            if (!successful) {
                ok = false;

                break;
            }

            result.Add(val, type);
            buff.clear();
        } else {
            buff.push_back(value[i]);

            if (value[i] == '[') {
                ++balance;
            } else if (value[i] == ']') {
                if (balance == 0) {
                    ok = false;

                    break;
                }

                --balance;
            }
        }
    }

    return {result, ok};
}

std::pair<std::any, bool> ConvertValue(const std::string& value, const omfl::Type& type) {
    using omfl::Type;

    if (type == Type::Integer) {
        return {std::stoi(value), true};
    } else if (type == Type::Float) {
        return {std::stod(value), true};
    } else if (type == Type::String) {
        return {value.substr(1, value.size() - 2), true};
    } else if (type == Type::Boolean) {
        if (value == "true") {
            return {true, true};
        }

        return {false, true};
    }

    assert(type == Type::Array);

    return ConstructValueArray(value);
}

void PrettifyString(std::string& str) {
    while (!str.empty() && str.back() == ' ') {
        str.pop_back();
    }

    size_t prefix_spaces = 0;

    for (; prefix_spaces < str.size(); ++prefix_spaces) {
        if (str[prefix_spaces] != ' ') {
            break;
        }
    }

    str = str.substr(prefix_spaces);
}

bool Update(omfl::Parser& parser, const std::vector<std::string>& current_sections, std::string& current_key, std::string& current_value) {
    PrettifyString(current_key);
    PrettifyString(current_value);
    
    if (current_key.empty() && current_value.empty()) {
        return true;
    }

    if (!CheckKeyValidity(current_key)) {
        return false;
    }

    omfl::Type value_type = GetValueType(current_value);

    if (value_type == omfl::Type::Undefined) {
        return false;
    }

    auto [converted_value, successful] = ConvertValue(current_value, value_type);

    if (!successful) {
        return false;
    }

    bool ok = parser.Add(current_sections, omfl::Item(current_key, converted_value, value_type));

    if (!ok) {
        return false;
    }

    current_key.clear();
    current_value.clear();

    return true;
}

std::pair<std::vector<std::string>, bool> ParseSections(std::string_view str, size_t& index) {
    std::vector<std::string> result;
    std::string buff;
    bool ok = true;

    for (; index < str.size() && str[index] != '\n'; ++index) {
        if (str[index] == '.' || str[index] == ']') {
            ok &= CheckKeyValidity(buff);
            result.emplace_back(buff);
            buff.clear();
        } else {
            buff += str[index];
        }
    }

    assert(buff.empty());

    return {result, ok};
}

std::pair<std::vector<std::string>, bool> ParseSections(std::ifstream& stream) {
    std::string buff;
    char character;

    for (; stream.get(character) && character != '\n';) {
        buff.push_back(character);
    }
    
    size_t starting_index = 0;

    return ParseSections(buff, starting_index);
}

omfl::Parser omfl::parse(const std::filesystem::path& path) {
    Parser parser;
    std::ifstream stream(path);

    if (!stream.is_open()) {
        throw std::runtime_error("No such file as " + path.filename().string());
    }

    std::vector<std::string> current_sections;
    std::string current_key;
    std::string current_value;
    bool equal_sign_seen = false;
    bool in_string = false;
    bool ignore = false;
    char character;

    for (; stream.get(character);) {
        if (character == '[' && !equal_sign_seen) {
            auto [current_sections_, successful] = ParseSections(stream);
            current_sections = current_sections_;

            if (!successful) {
                parser.MarkUnsuccessful();

                break;
            }

            current_key.clear();
            current_value.clear();

            continue;
        }
        
        if (character == '\n') {
            equal_sign_seen = false;
            in_string = false;
            ignore = false;

            if (!Update(parser, current_sections, current_key, current_value)) {
                parser.MarkUnsuccessful();

                break;
            }

            continue;
        }

        if (character == '#' && !in_string) {
            ignore = true;
        }

        if (ignore) {
            continue;
        }

        if (character == '=' && !in_string) {
            if (equal_sign_seen) {
                parser.MarkUnsuccessful();
                
                break;
            }

            equal_sign_seen = true;

            continue;
        }

        if (!equal_sign_seen) {
            current_key.push_back(character);
        } else {
            if (character == '\"') {
                in_string ^= 1;
            }

            current_value.push_back(character);
        }
    }

    if ((!current_key.empty() || !current_value.empty()) && parser.valid()) {
        if (!Update(parser, current_sections, current_key, current_value)) {
            parser.MarkUnsuccessful();
        }
    }

    return parser;
}

omfl::Parser omfl::parse(const std::string& str) {
    Parser parser;

    std::vector<std::string> current_sections;
    std::string current_key;
    std::string current_value;
    bool equal_sign_seen = false;
    bool in_string = false;
    bool ignore = false;

    for (size_t index = 0; index < str.size(); ++index) {
        char character = str[index];

        if (character == '[' && !equal_sign_seen) {
            auto [current_sections_, successful] = ParseSections(str, ++index);
            current_sections = current_sections_;

            if (!successful) {
                parser.MarkUnsuccessful();

                break;
            }

            current_key.clear();
            current_value.clear();

            continue;
        }
        
        if (character == '\n') {
            equal_sign_seen = false;
            in_string = false;
            ignore = false;

            if (!Update(parser, current_sections, current_key, current_value)) {
                parser.MarkUnsuccessful();

                break;
            }

            continue;
        }

        if (character == '#' && !in_string) {
            ignore = true;
        }

        if (ignore) {
            continue;
        }

        if (character == '=' && !in_string) {
            if (equal_sign_seen) {
                parser.MarkUnsuccessful();
                
                break;
            }

            equal_sign_seen = true;

            continue;
        }

        if (!equal_sign_seen) {
            current_key.push_back(character);
        } else {
            if (character == '\"') {
                in_string ^= 1;
            }

            current_value.push_back(character);
        }
    }

    if ((!current_key.empty() || !current_value.empty()) && parser.valid()) {
        if (!Update(parser, current_sections, current_key, current_value)) {
            parser.MarkUnsuccessful();
        }
    }

    return parser;
}
