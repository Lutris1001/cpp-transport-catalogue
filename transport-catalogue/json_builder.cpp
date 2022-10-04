
#include <string>
#include <variant>
#include <stdexcept>

#include "json.h"
#include "json_builder.h"

namespace json {

    using namespace std::literals;

    Builder::KeyItemContext Builder::BaseContext::Key(std::string key) {

        if (builder_ref_.nodes_stack_.size() == 1 && builder_ref_.keys_.empty() &&
            builder_ref_.commands_.empty() && builder_ref_.open_counter_ == 0) {
            throw std::logic_error("except_build_error"s);
        }

        if (!builder_ref_.nodes_stack_.back()->IsDict()) {
            throw std::logic_error("key_error_not_a_dict"s);
        }

        if (builder_ref_.commands_.back() == KEY) {
            throw std::logic_error("double_key_error"s);
        }

        builder_ref_.commands_.emplace_back(KEY);
        builder_ref_.keys_.emplace_back(key);

        return KeyItemContext(builder_ref_);
    }

    Builder::BaseContext Builder::BaseContext::Value(Node::Value value) {

        if (builder_ref_.nodes_stack_.size() == 1 && builder_ref_.keys_.empty() &&
            builder_ref_.commands_.empty() && builder_ref_.open_counter_ == 0) {
            throw std::logic_error("except_build_error"s);
        }

        if (builder_ref_.nodes_stack_.empty()) {
            auto tmp = new Node();
            tmp->SetValue() = value;
            builder_ref_.nodes_stack_.emplace_back(tmp);
            builder_ref_.state_ = GOOD;
            return BaseContext(builder_ref_);
        }

        if (!(builder_ref_.nodes_stack_.back()->IsDict() || builder_ref_.nodes_stack_.back()->IsArray())) {
            throw std::logic_error("double_value_in_dict"s);
        }

        if (builder_ref_.nodes_stack_.back()->IsDict()) {

            if (builder_ref_.commands_.back() != KEY) {
                throw std::logic_error("double_value_in_dict"s);
            }

            if (std::holds_alternative<nullptr_t>(value)) {
                std::get<Dict>(builder_ref_.nodes_stack_.back()->SetValue())[builder_ref_.keys_.back()] = Node();
            }
            if (std::holds_alternative<Array>(value)) {
                std::get<Dict>(builder_ref_.nodes_stack_.back()->SetValue())[builder_ref_.keys_.back()] = Node(std::get<Array>(value));
            }
            if (std::holds_alternative<Dict>(value)) {
                std::get<Dict>(builder_ref_.nodes_stack_.back()->SetValue())[builder_ref_.keys_.back()] = Node(std::get<Dict>(value));
            }
            if (std::holds_alternative<bool>(value)) {
                std::get<Dict>(builder_ref_.nodes_stack_.back()->SetValue())[builder_ref_.keys_.back()] = Node(std::get<bool>(value));
            }
            if (std::holds_alternative<int>(value)) {
                std::get<Dict>(builder_ref_.nodes_stack_.back()->SetValue())[builder_ref_.keys_.back()] = Node(std::get<int>(value));
            }
            if (std::holds_alternative<double>(value)) {
                std::get<Dict>(builder_ref_.nodes_stack_.back()->SetValue())[builder_ref_.keys_.back()] = Node(std::get<double>(value));
            }
            if (std::holds_alternative<std::string>(value)) {
                std::get<Dict>(builder_ref_.nodes_stack_.back()->SetValue())[builder_ref_.keys_.back()] = Node(std::get<std::string>(value));

            }

            builder_ref_.keys_.pop_back();
            builder_ref_.commands_.pop_back();
            return BaseContext(builder_ref_);
        }

        if (builder_ref_.nodes_stack_.back()->IsArray()) {

            if (builder_ref_.commands_.back() != ARRAY) {
                throw std::logic_error("double_value_in_dict"s);
            }

            if (std::holds_alternative<nullptr_t>(value)) {
                std::get<Array>(builder_ref_.nodes_stack_.back()->SetValue()).emplace_back(Node());
            }
            if (std::holds_alternative<Array>(value)) {
                std::get<Array>(builder_ref_.nodes_stack_.back()->SetValue()).emplace_back(std::get<Array>(value));
            }
            if (std::holds_alternative<Dict>(value)) {
                std::get<Array>(builder_ref_.nodes_stack_.back()->SetValue()).emplace_back(std::get<Dict>(value));
            }
            if (std::holds_alternative<bool>(value)) {
                std::get<Array>(builder_ref_.nodes_stack_.back()->SetValue()).emplace_back(std::get<bool>(value));
            }
            if (std::holds_alternative<int>(value)) {
                std::get<Array>(builder_ref_.nodes_stack_.back()->SetValue()).emplace_back(std::get<int>(value));
            }
            if (std::holds_alternative<double>(value)) {
                std::get<Array>(builder_ref_.nodes_stack_.back()->SetValue()).emplace_back(std::get<double>(value));
            }
            if (std::holds_alternative<std::string>(value)) {
                std::get<Array>(builder_ref_.nodes_stack_.back()->SetValue()).emplace_back(std::get<std::string>(value));
            }
            return BaseContext(builder_ref_);
        }

        return BaseContext(builder_ref_);
    }

    Builder::StartDictContext Builder::BaseContext::StartDict() {

        if (builder_ref_.nodes_stack_.size() == 1 && builder_ref_.keys_.empty() &&
            builder_ref_.commands_.empty() && builder_ref_.open_counter_ == 0) {
            throw std::logic_error("except_build_error"s);
        }

        if (!(builder_ref_.state_ == INIT || builder_ref_.commands_.back() == KEY ||
              builder_ref_.commands_.back() == ARRAY)) {
            throw std::logic_error("StartDict_error"s);
        }

        builder_ref_.nodes_stack_.push_back(new Node(Dict()));
        builder_ref_.commands_.emplace_back(DICT);
        builder_ref_.state_ = BAD;
        ++builder_ref_.open_counter_;
        return StartDictContext(builder_ref_);
    }

    Builder::ArrayContext Builder::BaseContext::StartArray() {

        if (builder_ref_.nodes_stack_.size() == 1 && builder_ref_.keys_.empty() &&
            builder_ref_.commands_.empty() && builder_ref_.open_counter_ == 0) {
            throw std::logic_error("except_build_error"s);
        }

        if (!(builder_ref_.state_ == INIT || builder_ref_.commands_.back() == KEY ||
              builder_ref_.commands_.back() == ARRAY)) {
            throw std::logic_error("StartArray_error"s);
        }

        builder_ref_.nodes_stack_.push_back(new Node(Array()));
        builder_ref_.commands_.emplace_back(ARRAY);
        builder_ref_.state_ = BAD;
        ++builder_ref_.open_counter_;
        return ArrayContext(builder_ref_);
    }

    Builder::BaseContext Builder::BaseContext::EndDict() {

        if (builder_ref_.nodes_stack_.size() == 1 && builder_ref_.keys_.empty() &&
            builder_ref_.commands_.empty() && builder_ref_.open_counter_ == 0) {
            throw std::logic_error("except_build_error"s);
        }

        if (builder_ref_.nodes_stack_.back()->IsArray()) {
            throw std::logic_error("end_dict_error"s);
        }

        if (builder_ref_.commands_.back() != DICT) {
            throw std::logic_error("end_dict_error_not_dict_last_command"s);
        }

        if (builder_ref_.nodes_stack_.size() == 1) {
            builder_ref_.commands_.pop_back();
            builder_ref_.state_ = GOOD;
            --builder_ref_.open_counter_;
            return BaseContext(builder_ref_);
        }

        if (builder_ref_.nodes_stack_.at(builder_ref_.nodes_stack_.size() - 2)->IsArray()) {
            std::get<Array>(builder_ref_.nodes_stack_[builder_ref_.nodes_stack_.size() - 2]->SetValue()).
                    emplace_back(*builder_ref_.nodes_stack_.back());
        }

        if (builder_ref_.nodes_stack_.at(builder_ref_.nodes_stack_.size() - 2)->IsDict()) {
            std::get<Dict>(builder_ref_.nodes_stack_[builder_ref_.nodes_stack_.size() - 2]->SetValue())
            [builder_ref_.keys_.back()] = *builder_ref_.nodes_stack_.back();
            builder_ref_.keys_.pop_back();
        }

        builder_ref_.nodes_stack_.pop_back();
        builder_ref_.commands_.pop_back();

        if (builder_ref_.commands_.back() == KEY) {
            builder_ref_.commands_.pop_back();
        }

        builder_ref_.state_ = GOOD;
        --builder_ref_.open_counter_;
        return BaseContext(builder_ref_);
    }

    Builder::BaseContext Builder::BaseContext::EndArray() {

        if (builder_ref_.nodes_stack_.size() == 1 && builder_ref_.keys_.empty() &&
            builder_ref_.commands_.empty() && builder_ref_.open_counter_ == 0) {
            throw std::logic_error("except_build_error"s);
        }

        if (builder_ref_.nodes_stack_.back()->IsDict()) {
            throw std::logic_error("end_array_error"s);
        }

        if (builder_ref_.commands_.back() != ARRAY) {
            throw std::logic_error("end_ARRAY_error_not_ARRAY_last_command"s);
        }

        if (builder_ref_.nodes_stack_.size() == 1) {
            builder_ref_.commands_.pop_back();
            builder_ref_.state_ = GOOD;
            --builder_ref_.open_counter_;
            return BaseContext(builder_ref_);
        }

        if (builder_ref_.nodes_stack_.at(builder_ref_.nodes_stack_.size() - 2)->IsArray()) {

            std::get<Array>(builder_ref_.nodes_stack_[builder_ref_.nodes_stack_.size() - 2]->SetValue()).
                    emplace_back(*builder_ref_.nodes_stack_.back());
        }

        if (builder_ref_.nodes_stack_.at(builder_ref_.nodes_stack_.size() - 2)->IsDict()) {

            std::get<Dict>(builder_ref_.nodes_stack_[builder_ref_.nodes_stack_.size() - 2]->SetValue())[builder_ref_.keys_.back()] = *builder_ref_.nodes_stack_.back();
            builder_ref_.keys_.pop_back();
        }

        builder_ref_.nodes_stack_.pop_back();
        builder_ref_.commands_.pop_back();

        if (builder_ref_.commands_.back() == KEY) {
            builder_ref_.commands_.pop_back();
        }

        builder_ref_.state_ = GOOD;
        --builder_ref_.open_counter_;
        return BaseContext(builder_ref_);
    }

    Node Builder::BaseContext::Build() {

        if (builder_ref_.nodes_stack_.size() > 1 || !builder_ref_.keys_.empty() || !builder_ref_.commands_.empty() ||
            builder_ref_.open_counter_ != 0 || builder_ref_.state_ != GOOD) {
            throw std::logic_error("build_error"s);
        }

        builder_ref_.root_ = *builder_ref_.nodes_stack_[0];
        builder_ref_.nodes_stack_.pop_back();

        return builder_ref_.root_;
    }

    Builder::DictValueContext Builder::KeyItemContext::Value(Node::Value value) {

        if (builder_ref_.nodes_stack_.size() == 1 && builder_ref_.keys_.empty() &&
            builder_ref_.commands_.empty() && builder_ref_.open_counter_ == 0) {
            throw std::logic_error("except_build_error"s);
        }

        if (!(builder_ref_.nodes_stack_.back()->IsDict() || builder_ref_.nodes_stack_.back()->IsArray())) {
            throw std::logic_error("double_value_in_dict"s);
        }

        if (builder_ref_.nodes_stack_.back()->IsDict()) {

            if (builder_ref_.commands_.back() != KEY) {
                throw std::logic_error("double_value_in_dict"s);
            }

            if (std::holds_alternative<nullptr_t>(value)) {
                std::get<Dict>(builder_ref_.nodes_stack_.back()->SetValue())[builder_ref_.keys_.back()] = Node();
            }
            if (std::holds_alternative<Array>(value)) {
                std::get<Dict>(builder_ref_.nodes_stack_.back()->SetValue())[builder_ref_.keys_.back()] = Node(std::get<Array>(value));
            }
            if (std::holds_alternative<Dict>(value)) {
                std::get<Dict>(builder_ref_.nodes_stack_.back()->SetValue())[builder_ref_.keys_.back()] = Node(std::get<Dict>(value));
            }
            if (std::holds_alternative<bool>(value)) {
                std::get<Dict>(builder_ref_.nodes_stack_.back()->SetValue())[builder_ref_.keys_.back()] = Node(std::get<bool>(value));
            }
            if (std::holds_alternative<int>(value)) {
                std::get<Dict>(builder_ref_.nodes_stack_.back()->SetValue())[builder_ref_.keys_.back()] = Node(std::get<int>(value));
            }
            if (std::holds_alternative<double>(value)) {
                std::get<Dict>(builder_ref_.nodes_stack_.back()->SetValue())[builder_ref_.keys_.back()] = Node(std::get<double>(value));
            }
            if (std::holds_alternative<std::string>(value)) {
                std::get<Dict>(builder_ref_.nodes_stack_.back()->SetValue())[builder_ref_.keys_.back()] = Node(std::get<std::string>(value));

            }

            builder_ref_.keys_.pop_back();
            builder_ref_.commands_.pop_back();

        }
        return DictValueContext(builder_ref_);
    }

    Builder::ArrayContext Builder::ArrayContext::Value(Node::Value value) {

        if (builder_ref_.nodes_stack_.size() == 1 && builder_ref_.keys_.empty() &&
            builder_ref_.commands_.empty() && builder_ref_.open_counter_ == 0) {
            throw std::logic_error("except_build_error"s);
        }

        if (builder_ref_.nodes_stack_.back()->IsArray()) {

            if (builder_ref_.commands_.back() != ARRAY) {
                throw std::logic_error("double_value_in_dict"s);
            }

            if (std::holds_alternative<nullptr_t>(value)) {
                std::get<Array>(builder_ref_.nodes_stack_.back()->SetValue()).emplace_back(Node());
                return ArrayContext(builder_ref_);
            }
            if (std::holds_alternative<Array>(value)) {
                std::get<Array>(builder_ref_.nodes_stack_.back()->SetValue()).emplace_back(std::get<Array>(value));
                return ArrayContext(builder_ref_);
            }
            if (std::holds_alternative<Dict>(value)) {
                std::get<Array>(builder_ref_.nodes_stack_.back()->SetValue()).emplace_back(std::get<Dict>(value));
                return ArrayContext(builder_ref_);
            }
            if (std::holds_alternative<bool>(value)) {
                std::get<Array>(builder_ref_.nodes_stack_.back()->SetValue()).emplace_back(std::get<bool>(value));
                return ArrayContext(builder_ref_);
            }
            if (std::holds_alternative<int>(value)) {
                std::get<Array>(builder_ref_.nodes_stack_.back()->SetValue()).emplace_back(std::get<int>(value));
                return ArrayContext(builder_ref_);
            }
            if (std::holds_alternative<double>(value)) {
                std::get<Array>(builder_ref_.nodes_stack_.back()->SetValue()).emplace_back(std::get<double>(value));
                return ArrayContext(builder_ref_);
            }
            if (std::holds_alternative<std::string>(value)) {
                std::get<Array>(builder_ref_.nodes_stack_.back()->SetValue()).emplace_back(std::get<std::string>(value));
                return ArrayContext(builder_ref_);
            }

        }

        return ArrayContext(builder_ref_);
    }

    Builder::AfterOnlyOneValueContext Builder::OnlyOneValueContext::Value(Node::Value value) {
        if (builder_ref_.nodes_stack_.size() == 1 && builder_ref_.keys_.empty() &&
            builder_ref_.commands_.empty() && builder_ref_.open_counter_ == 0) {
            throw std::logic_error("except_build_error"s);
        }

        if (builder_ref_.nodes_stack_.empty()) {
            auto tmp = new Node();
            tmp->SetValue() = value;
            builder_ref_.nodes_stack_.emplace_back(tmp);
            builder_ref_.state_ = GOOD;
            return AfterOnlyOneValueContext(builder_ref_);
        }
        return AfterOnlyOneValueContext(builder_ref_);
    }

} // end namespace json