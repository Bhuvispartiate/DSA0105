#pragma once

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <stdexcept>
#include <cstdint>
#include <iomanip>

namespace SecureVault {

enum class JsonType {
    Null,
    Bool,
    Number,
    String,
    Array,
    Object
};

class JsonValue {
public:
    JsonType type;
    bool boolVal = false;
    double numVal = 0.0;
    std::string strVal;
    std::vector<JsonValue> arrVal;
    std::map<std::string, JsonValue> objVal;

    JsonValue() : type(JsonType::Null) {}
    JsonValue(std::nullptr_t) : type(JsonType::Null) {}
    JsonValue(bool b) : type(JsonType::Bool), boolVal(b) {}
    JsonValue(int n) : type(JsonType::Number), numVal(n) {}
    JsonValue(unsigned int n) : type(JsonType::Number), numVal(n) {}
    JsonValue(int64_t n) : type(JsonType::Number), numVal(static_cast<double>(n)) {}
    JsonValue(uint64_t n) : type(JsonType::Number), numVal(static_cast<double>(n)) {}
    JsonValue(double n) : type(JsonType::Number), numVal(n) {}
    JsonValue(const char* s) : type(JsonType::String), strVal(s) {}
    JsonValue(const std::string& s) : type(JsonType::String), strVal(s) {}
    JsonValue(const std::vector<JsonValue>& a) : type(JsonType::Array), arrVal(a) {}
    JsonValue(const std::map<std::string, JsonValue>& o) : type(JsonType::Object), objVal(o) {}

    static JsonValue makeArray() {
        JsonValue v;
        v.type = JsonType::Array;
        return v;
    }

    static JsonValue makeObject() {
        JsonValue v;
        v.type = JsonType::Object;
        return v;
    }

    bool isNull() const { return type == JsonType::Null; }
    bool isBool() const { return type == JsonType::Bool; }
    bool isNumber() const { return type == JsonType::Number; }
    bool isString() const { return type == JsonType::String; }
    bool isArray() const { return type == JsonType::Array; }
    bool isObject() const { return type == JsonType::Object; }

    const std::string& asString(const std::string& def = "") const {
        return isString() ? strVal : def;
    }

    double asNumber(double def = 0.0) const {
        return isNumber() ? numVal : def;
    }

    int64_t asInt64(int64_t def = 0) const {
        return isNumber() ? static_cast<int64_t>(numVal) : def;
    }

    uint32_t asUint32(uint32_t def = 0) const {
        return isNumber() ? static_cast<uint32_t>(numVal) : def;
    }

    bool asBool(bool def = false) const {
        return isBool() ? boolVal : def;
    }

    const std::vector<JsonValue>& asArray() const {
        static const std::vector<JsonValue> emptyArr;
        return isArray() ? arrVal : emptyArr;
    }

    const std::map<std::string, JsonValue>& asObject() const {
        static const std::map<std::string, JsonValue> emptyObj;
        return isObject() ? objVal : emptyObj;
    }

    bool has(const std::string& key) const {
        return isObject() && objVal.find(key) != objVal.end();
    }

    JsonValue& operator[](const std::string& key) {
        if (type != JsonType::Object) {
            type = JsonType::Object;
            objVal.clear();
        }
        return objVal[key];
    }

    const JsonValue& operator[](const std::string& key) const {
        static const JsonValue nullVal;
        if (!isObject()) return nullVal;
        auto it = objVal.find(key);
        if (it != objVal.end()) return it->second;
        return nullVal;
    }

    void push_back(const JsonValue& val) {
        if (type != JsonType::Array) {
            type = JsonType::Array;
            arrVal.clear();
        }
        arrVal.push_back(val);
    }

    std::string dump() const {
        std::ostringstream ss;
        serializeInternal(ss);
        return ss.str();
    }

    static JsonValue parse(const std::string& str) {
        size_t idx = 0;
        skipWhitespace(str, idx);
        return parseValue(str, idx);
    }

private:
    void serializeInternal(std::ostringstream& ss) const {
        switch (type) {
            case JsonType::Null: ss << "null"; break;
            case JsonType::Bool: ss << (boolVal ? "true" : "false"); break;
            case JsonType::Number: {
                if (numVal == static_cast<int64_t>(numVal)) {
                    ss << static_cast<int64_t>(numVal);
                } else {
                    ss << numVal;
                }
                break;
            }
            case JsonType::String: {
                ss << '"';
                for (char c : strVal) {
                    if (c == '"') ss << "\\\"";
                    else if (c == '\\') ss << "\\\\";
                    else if (c == '\b') ss << "\\b";
                    else if (c == '\f') ss << "\\f";
                    else if (c == '\n') ss << "\\n";
                    else if (c == '\r') ss << "\\r";
                    else if (c == '\t') ss << "\\t";
                    else ss << c;
                }
                ss << '"';
                break;
            }
            case JsonType::Array: {
                ss << '[';
                for (size_t i = 0; i < arrVal.size(); ++i) {
                    if (i > 0) ss << ',';
                    arrVal[i].serializeInternal(ss);
                }
                ss << ']';
                break;
            }
            case JsonType::Object: {
                ss << '{';
                size_t i = 0;
                for (const auto& pair : objVal) {
                    if (i++ > 0) ss << ',';
                    ss << '"' << pair.first << "\":";
                    pair.second.serializeInternal(ss);
                }
                ss << '}';
                break;
            }
        }
    }

    static void skipWhitespace(const std::string& s, size_t& i) {
        while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r')) {
            i++;
        }
    }

    static JsonValue parseValue(const std::string& s, size_t& i) {
        skipWhitespace(s, i);
        if (i >= s.size()) return JsonValue();

        char c = s[i];
        if (c == '{') return parseObject(s, i);
        if (c == '[') return parseArray(s, i);
        if (c == '"') return parseString(s, i);
        if (c == 't' || c == 'f') return parseBool(s, i);
        if (c == 'n') return parseNull(s, i);
        if (c == '-' || (c >= '0' && c <= '9')) return parseNumber(s, i);

        throw std::runtime_error("Unexpected character in JSON: " + std::string(1, c));
    }

    static JsonValue parseObject(const std::string& s, size_t& i) {
        i++; // skip '{'
        JsonValue obj = JsonValue::makeObject();
        skipWhitespace(s, i);
        if (i < s.size() && s[i] == '}') {
            i++;
            return obj;
        }

        while (i < s.size()) {
            skipWhitespace(s, i);
            if (i >= s.size() || s[i] != '"') throw std::runtime_error("Expected string key in object");
            std::string key = parseString(s, i).strVal;
            skipWhitespace(s, i);
            if (i >= s.size() || s[i] != ':') throw std::runtime_error("Expected ':' after key");
            i++; // skip ':'
            obj[key] = parseValue(s, i);
            skipWhitespace(s, i);
            if (i < s.size() && s[i] == ',') {
                i++;
                continue;
            }
            if (i < s.size() && s[i] == '}') {
                i++;
                break;
            }
            throw std::runtime_error("Expected ',' or '}' in object");
        }
        return obj;
    }

    static JsonValue parseArray(const std::string& s, size_t& i) {
        i++; // skip '['
        JsonValue arr = JsonValue::makeArray();
        skipWhitespace(s, i);
        if (i < s.size() && s[i] == ']') {
            i++;
            return arr;
        }

        while (i < s.size()) {
            arr.push_back(parseValue(s, i));
            skipWhitespace(s, i);
            if (i < s.size() && s[i] == ',') {
                i++;
                continue;
            }
            if (i < s.size() && s[i] == ']') {
                i++;
                break;
            }
            throw std::runtime_error("Expected ',' or ']' in array");
        }
        return arr;
    }

    static JsonValue parseString(const std::string& s, size_t& i) {
        i++; // skip initial '"'
        std::string res;
        while (i < s.size()) {
            char c = s[i++];
            if (c == '"') {
                return JsonValue(res);
            }
            if (c == '\\' && i < s.size()) {
                char esc = s[i++];
                if (esc == '"') res += '"';
                else if (esc == '\\') res += '\\';
                else if (esc == '/') res += '/';
                else if (esc == 'b') res += '\b';
                else if (esc == 'f') res += '\f';
                else if (esc == 'n') res += '\n';
                else if (esc == 'r') res += '\r';
                else if (esc == 't') res += '\t';
                else res += esc;
            } else {
                res += c;
            }
        }
        throw std::runtime_error("Unterminated string in JSON");
    }

    static JsonValue parseNumber(const std::string& s, size_t& i) {
        size_t start = i;
        if (s[i] == '-') i++;
        while (i < s.size() && ((s[i] >= '0' && s[i] <= '9') || s[i] == '.' || s[i] == 'e' || s[i] == 'E' || s[i] == '+' || s[i] == '-')) {
            i++;
        }
        double val = std::stod(s.substr(start, i - start));
        return JsonValue(val);
    }

    static JsonValue parseBool(const std::string& s, size_t& i) {
        if (s.substr(i, 4) == "true") {
            i += 4;
            return JsonValue(true);
        }
        if (s.substr(i, 5) == "false") {
            i += 5;
            return JsonValue(false);
        }
        throw std::runtime_error("Invalid boolean literal in JSON");
    }

    static JsonValue parseNull(const std::string& s, size_t& i) {
        if (s.substr(i, 4) == "null") {
            i += 4;
            return JsonValue(nullptr);
        }
        throw std::runtime_error("Invalid null literal in JSON");
    }
};

} // namespace SecureVault

