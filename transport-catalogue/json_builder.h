#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>
#include <stdexcept>

#include "json.h"

namespace json {
    
class Builder {

    class BaseContext;
    class DictContext;
    class ArrayContext;
    class KeyContext;

public:

    KeyContext Key(std::string key);
    BaseContext Value(Node::Value value);
    DictContext StartDict();
    ArrayContext StartArray();
    BaseContext EndDict();
    BaseContext EndArray();
    Node Build();

private:
    
    Node root_;
    std::vector<Node*> nodes_stack_;
    std::vector<std::string> keys_;

    void PasteValueInDict(Node::Value value);
    void PasteValueInArray(Node::Value value);
    BaseContext UniversalEnd();

    struct BaseContext {
    public:
        explicit BaseContext(Builder& ref)
                : builder_ref_(ref)
        {
        }

        KeyContext Key(std::string key);
        BaseContext Value(Node::Value value);
        DictContext StartDict();
        ArrayContext StartArray();
        BaseContext EndDict();
        BaseContext EndArray();
        Node Build();

        Builder& builder_ref_;
    };

    class ArrayContext final : public BaseContext {
    public:
        explicit ArrayContext(Builder& ref)
                : BaseContext(ref)
        {
        }

        // Only Value, StartDict, StartArray or EndArray can be called
        KeyContext Key(std::string key) = delete;
        BaseContext EndDict() = delete;
        Node Build() = delete;

        ArrayContext Value(Node::Value value);
    };

    class KeyContext final : public BaseContext {
    public:
        explicit KeyContext(Builder& ref)
                : BaseContext(ref)
        {
        }

        // Only Value, StartDict or StartArray can be called
        KeyContext Key(std::string key) = delete;
        BaseContext EndDict() = delete;
        BaseContext EndArray() = delete;
        Node Build() = delete;

        DictContext Value(Node::Value value);

    };

    class DictContext final : public BaseContext {
    public:
        explicit DictContext(Builder& ref)
                : BaseContext(ref)
        {
        }

        // Only Key or DictEnd() can be called
        DictContext StartDict() = delete;
        ArrayContext StartArray() = delete;
        BaseContext Value(Node::Value value) = delete;
        BaseContext EndArray() = delete;
        Node Build() = delete;
    };

};
    
} // end namespace json