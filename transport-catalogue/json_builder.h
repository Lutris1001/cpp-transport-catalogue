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
    class StartDictContext;
    class ArrayContext;
    class KeyItemContext;
    class DictValueContext;
    class OnlyOneValueContext;
    class AfterOnlyOneValueContext;

public:

    AfterOnlyOneValueContext Value(Node::Value value) {
        return OnlyOneValueContext(*this).Value(value);
    }
    StartDictContext StartDict() {
        return BaseContext(*this).StartDict();
    }
    ArrayContext StartArray() {
        return BaseContext(*this).StartArray();
    }

private:

    enum State {
        INIT,
        BAD,
        GOOD,
        ARRAY,
        DICT,
        KEY
    };
    
    Node root_;
    std::vector<Node*> nodes_stack_;
    std::vector<State> commands_;
    State state_ = INIT;
    int open_counter_ = 0;
    std::vector<std::string> keys_;

    struct BaseContext {
    public:
        explicit BaseContext(Builder& ref)
                : builder_ref_(ref)
        {
        }

        KeyItemContext Key(std::string key);
        BaseContext Value(Node::Value value);
        StartDictContext StartDict();
        ArrayContext StartArray();
        BaseContext EndDict();
        BaseContext EndArray();
        Node Build();

        Builder& builder_ref_;
    };


    struct StartDictContext final : public BaseContext {
        explicit StartDictContext(Builder& ref)
                : BaseContext(ref)
        {
        }

        StartDictContext StartDict() = delete;
        ArrayContext StartArray() = delete;
        BaseContext EndArray() = delete;
        Node Build() = delete;

    };

    struct ArrayContext final : public BaseContext {
    public:
        explicit ArrayContext(Builder& ref)
                : BaseContext(ref)
        {
        }

        KeyItemContext Key(std::string key) = delete;
        BaseContext EndDict() = delete;
        Node Build() = delete;

        ArrayContext Value(Node::Value value);
    };
    struct KeyItemContext final : public BaseContext {
    public:
        explicit KeyItemContext(Builder& ref)
                : BaseContext(ref)
        {
        }

        KeyItemContext Key(std::string key) = delete;
        BaseContext EndDict() = delete;
        BaseContext EndArray() = delete;
        Node Build() = delete;

        DictValueContext Value(Node::Value value);

    };

    struct DictValueContext final : public BaseContext {
    public:
        explicit DictValueContext(Builder& ref)
                : BaseContext(ref)
        {
        }
        StartDictContext StartDict() = delete;
        ArrayContext StartArray() = delete;
        BaseContext Value(Node::Value value) = delete;
        BaseContext EndArray() = delete;
        Node Build() = delete;
    };

    struct ArrayValueContext final : public BaseContext {
    public:
        explicit ArrayValueContext(Builder& ref)
                : BaseContext(ref)
        {
        }

        KeyItemContext Key(std::string key) = delete;
        BaseContext EndDict() = delete;
        Node Build() = delete;
    };

    struct OnlyOneValueContext final : public BaseContext {
    public:
        explicit OnlyOneValueContext(Builder& ref)
                : BaseContext(ref)
        {
        }

        KeyItemContext Key(std::string key) = delete;
        StartDictContext StartDict() = delete;
        ArrayContext StartArray() = delete;
        BaseContext EndDict() = delete;
        BaseContext EndArray() = delete;
        Node Build() = delete;

        AfterOnlyOneValueContext Value(Node::Value value);
    };

    struct AfterOnlyOneValueContext final : public BaseContext {
    public:
        explicit AfterOnlyOneValueContext(Builder& ref)
                : BaseContext(ref)
        {
        }

        KeyItemContext Key(std::string key) = delete;
        StartDictContext StartDict() = delete;
        ArrayContext StartArray() = delete;
        BaseContext EndDict() = delete;
        BaseContext EndArray() = delete;
    };

};
    
} // end namespace json