
#include <string>
#include <variant>
#include <stdexcept>

#include "json.h"
#include "json_builder.h"

namespace json {

    using namespace std::literals;

    void Builder::PasteValueInDict(Node::Value value) {

        if (std::holds_alternative<nullptr_t>(value)) {
            std::get<Dict>(nodes_stack_.back()->SetValue())[keys_.back()] = Node();
        }
        if (std::holds_alternative<Array>(value)) {
            std::get<Dict>(nodes_stack_.back()->SetValue())[keys_.back()] = Node(std::get<Array>(value));
        }
        if (std::holds_alternative<Dict>(value)) {
            std::get<Dict>(nodes_stack_.back()->SetValue())[keys_.back()] = Node(std::get<Dict>(value));
        }
        if (std::holds_alternative<bool>(value)) {
            std::get<Dict>(nodes_stack_.back()->SetValue())[keys_.back()] = Node(std::get<bool>(value));
        }
        if (std::holds_alternative<int>(value)) {
            std::get<Dict>(nodes_stack_.back()->SetValue())[keys_.back()] = Node(std::get<int>(value));
        }
        if (std::holds_alternative<double>(value)) {
            std::get<Dict>(nodes_stack_.back()->SetValue())[keys_.back()] = Node(std::get<double>(value));
        }
        if (std::holds_alternative<std::string>(value)) {
            std::get<Dict>(nodes_stack_.back()->SetValue())[keys_.back()] = Node(std::get<std::string>(value));
        }

        keys_.pop_back();
    }

    void Builder::PasteValueInArray(Node::Value value) {

        if (std::holds_alternative<nullptr_t>(value)) {
            std::get<Array>(nodes_stack_.back()->SetValue()).emplace_back(Node());
        }
        if (std::holds_alternative<Array>(value)) {
            std::get<Array>(nodes_stack_.back()->SetValue()).emplace_back(std::get<Array>(value));
        }
        if (std::holds_alternative<Dict>(value)) {
            std::get<Array>(nodes_stack_.back()->SetValue()).emplace_back(std::get<Dict>(value));
        }
        if (std::holds_alternative<bool>(value)) {
            std::get<Array>(nodes_stack_.back()->SetValue()).emplace_back(std::get<bool>(value));
        }
        if (std::holds_alternative<int>(value)) {
            std::get<Array>(nodes_stack_.back()->SetValue()).emplace_back(std::get<int>(value));
        }
        if (std::holds_alternative<double>(value)) {
            std::get<Array>(nodes_stack_.back()->SetValue()).emplace_back(std::get<double>(value));
        }
        if (std::holds_alternative<std::string>(value)) {
            std::get<Array>(nodes_stack_.back()->SetValue()).emplace_back(std::get<std::string>(value));
        }

    }

    Builder::BaseContext Builder::UniversalEnd() {

        if (nodes_stack_.size() == 1) {
            return BaseContext(*this);
        }

        if (nodes_stack_.at(nodes_stack_.size() - 2)->IsArray()) {
            std::get<Array>(nodes_stack_[nodes_stack_.size() - 2]->SetValue()).
                    emplace_back(*nodes_stack_.back());
        }

        if (nodes_stack_.at(nodes_stack_.size() - 2)->IsDict()) {
            std::get<Dict>(nodes_stack_[nodes_stack_.size() - 2]->SetValue())[keys_.back()] = *nodes_stack_.back();
            keys_.pop_back();
        }

        nodes_stack_.pop_back();
        return BaseContext(*this);
    }

    Builder::KeyContext Builder::Key(std::string key) {

        if (!nodes_stack_.back()->IsDict()) {
            throw std::logic_error("key_error_not_a_dict"s);
        }

        keys_.emplace_back(key);
        return KeyContext(*this);
    }

    Builder::BaseContext Builder::Value(Node::Value value) {

        if (nodes_stack_.empty()) {
            auto tmp = new Node();
            tmp->SetValue() = value;
            nodes_stack_.emplace_back(tmp);
            return BaseContext(*this);
        }

        if (!(nodes_stack_.back()->IsDict() || nodes_stack_.back()->IsArray())) {
            throw std::logic_error("double_value_in_dict"s);
        }

        if (nodes_stack_.back()->IsDict()) {
            PasteValueInDict(value);
            return BaseContext(*this);
        }

        if (nodes_stack_.back()->IsArray()) {
            PasteValueInArray(value);
            return BaseContext(*this);
        }

        return BaseContext(*this);
    }

    Builder::DictContext Builder::StartDict() {
        nodes_stack_.push_back(new Node(Dict()));
        return DictContext(*this);
    }

    Builder::ArrayContext Builder::StartArray() {
        nodes_stack_.push_back(new Node(Array()));
        return ArrayContext(*this);
    }

    Builder::BaseContext Builder::EndDict() {

        if (nodes_stack_.back()->IsArray()) {
            throw std::logic_error("end_dict_error"s);
        }

        return UniversalEnd();
    }

    Builder::BaseContext Builder::EndArray() {

        if (nodes_stack_.back()->IsDict()) {
            throw std::logic_error("end_array_error"s);
        }

        return UniversalEnd();
    }

    Node Builder::Build() {

        if (nodes_stack_.size() > 1 || !keys_.empty()) {
            throw std::logic_error("build_error"s);
        }

        root_ = std::move(*nodes_stack_[0]);
        nodes_stack_.pop_back();
        return root_;
    }

    // -------------------------------

    Builder::DictContext Builder::KeyContext::Value(Node::Value value) {

        if (!builder_ref_.nodes_stack_.back()->IsDict()) {
            throw std::logic_error("double_value_in_dict"s);
        }

        builder_ref_.PasteValueInDict(value);
        return DictContext(builder_ref_);
    }

    Builder::ArrayContext Builder::ArrayContext::Value(Node::Value value) {

        if (!builder_ref_.nodes_stack_.back()->IsArray()) {
            throw std::logic_error("array_error"s);
        }

        builder_ref_.PasteValueInArray(value);
        return ArrayContext(builder_ref_);
    }

    // -------------------------------

    Builder::KeyContext Builder::BaseContext::Key(std::string key) {
        return builder_ref_.Key(key);
    }

    Builder::BaseContext Builder::BaseContext::Value(Node::Value value) {
        return builder_ref_.Value(value);
    }

    Builder::DictContext Builder::BaseContext::StartDict() {
        return builder_ref_.StartDict();
    }

    Builder::ArrayContext Builder::BaseContext::StartArray() {
        return builder_ref_.StartArray();
    }

    Builder::BaseContext Builder::BaseContext::EndDict() {
        return builder_ref_.EndDict();
    }

    Builder::BaseContext Builder::BaseContext::EndArray() {
        return builder_ref_.EndArray();
    }

    Node Builder::BaseContext::Build() {
        return builder_ref_.Build();
    }

} // end namespace json