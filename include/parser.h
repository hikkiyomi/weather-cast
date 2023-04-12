#pragma once

#include <any>
#include <cinttypes>
#include <filesystem>
#include <istream>
#include <map>
#include <string_view>
#include <utility>
#include <vector>

namespace omfl {
    enum Type {
        Undefined,
        Integer,
        Float,
        String,
        Boolean,
        Array,
        Section
    };

    class Item {
    public:
        explicit Item(std::string_view _key, const std::any& _value, Type _value_type);

        const std::string& GetKey() const;
        std::any& GetValue();
        const Type& GetType() const;

        const Item& Get(std::string_view name) const;
        const Item& Get(const std::vector<std::string_view>& way, size_t index) const;
        
        bool IsInt() const;
        int32_t AsInt() const;
        int32_t AsIntOrDefault(int32_t value) const;

        bool IsFloat() const;
        double AsFloat() const;
        double AsFloatOrDefault(double value) const;

        bool IsString() const;
        std::string_view AsString() const;
        std::string_view AsStringOrDefault(std::string_view value) const;

        bool IsBool() const;
        bool AsBool() const;
        bool AsBoolOrDefault(bool value) const;

        bool IsArray() const;
        const Item& operator[](size_t index) const;
    private:
        std::string key;
        std::any value;
        Type value_type = Type::Undefined;
    };

    class ValueArray {
    public:
        ValueArray();

        void Add(const std::any& value, Type type);
        const Item& Get(size_t index) const;
    private:
        std::vector<Item> values_;
        Item trash_item_;
    };

    class Parser {
    public:
        bool valid() const;
        void MarkUnsuccessful();

        bool Add(const std::vector<std::string>& section_way, const Item& appending_item);
        const Item& Get(std::string_view name) const;
    private:
        class Trie {
        public:
            Trie();
        
            bool AddItem(const std::vector<std::string>& section_way, const Item& appending_item);
            const Item& GetItem(std::string_view name) const;
        private:
            Item root_;
        } tree_;

        bool successful_parse_ = true;
    };

    Parser parse(const std::filesystem::path& path);
    Parser parse(const std::string& str);
}
