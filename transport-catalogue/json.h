#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:

    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
    
    template<typename T>
    Node(const T& some);
    
    Node() = default;
    
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;
    
    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;
    
    const Value& GetValue() const { return value_; }

    bool operator==(const Node& other) const {
        return this->value_ == other.value_;
    }
    
    bool operator!=(const Node& other) const {
        return !(this->value_ == other.value_);
    }
    
private:
    Value value_ = std::nullptr_t{};
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

    bool operator==(const Document& other) const {
        return GetRoot() == other.GetRoot();
    }
    
    bool operator!=(const Document& other) const {
        return GetRoot() != other.GetRoot();
    }
    
private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

template<typename T>
Node::Node(const T& some) {
    value_ = some;
}
    
}  // namespace json


