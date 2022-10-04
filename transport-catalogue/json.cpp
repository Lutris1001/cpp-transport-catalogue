#include "json.h"
#include <variant>
#include <string>

using namespace std;

namespace json {

    bool Node::IsInt() const {
        return std::holds_alternative<int>(*this);
    }

    bool Node::IsDouble() const {
        return std::holds_alternative<int>(*this) || std::holds_alternative<double>(*this);
    }

    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(*this);
    }

    bool Node::IsBool() const {
        return std::holds_alternative<bool>(*this);
    }

    bool Node::IsString() const {
        return std::holds_alternative<std::string>(*this);
    }

    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(*this);
    }

    bool Node::IsArray() const {
        return std::holds_alternative<Array>(*this);
    }

    bool Node::IsDict() const {
        return std::holds_alternative<Dict>(*this);
    }


    int Node::AsInt() const {
        if (!IsInt()) {
            throw std::logic_error("Logic_error"s);
        }
        return std::get<int>(*this);
    }

    bool Node::AsBool() const {
        if (!IsBool()) {
            throw std::logic_error("Logic_error"s);
        }
        return std::get<bool>(*this);
    }

    double Node::AsDouble() const {
        if (!(IsInt() || IsDouble())) {
            throw std::logic_error("Logic_error"s);
        }
        if (IsInt()) {
            return static_cast<double>(std::get<int>(*this));
        }
        return std::get<double>(*this);
    }

    const std::string& Node::AsString() const {
        if (!IsString()) {
            throw std::logic_error("Logic_error"s);
        }
        return std::get<std::string>(*this);
    }

    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw std::logic_error("Logic_error"s);
        }
        return std::get<Array>(*this);
    }

    const Dict& Node::AsDict() const {
        if (!IsDict()) {
            throw std::logic_error("Logic_error"s);
        }
        return std::get<Dict>(*this);
    }

// ---------------- NODE LOAD PARSING -----------------

    using Number = std::variant<int, double>;

    Number LoadNumber(std::istream& input) {
        using namespace std::literals;

        std::string parsed_num;

        // Считывает в parsed_num очередной символ из input
        auto read_char = [&parsed_num, &input] {
            parsed_num += static_cast<char>(input.get());
            if (!input) {
                throw ParsingError("Failed to read number from stream"s);
            }
        };

        // Считывает одну или более цифр в parsed_num из input
        auto read_digits = [&input, read_char] {
            if (!std::isdigit(input.peek())) {
                throw ParsingError("A digit is expected"s);
            }
            while (std::isdigit(input.peek())) {
                read_char();
            }
        };

        if (input.peek() == '-') {
            read_char();
        }
        // Парсим целую часть числа
        if (input.peek() == '0') {
            read_char();
            // После 0 в JSON не могут идти другие цифры
        } else {
            read_digits();
        }

        bool is_int = true;
        // Парсим дробную часть числа
        if (input.peek() == '.') {
            read_char();
            read_digits();
            is_int = false;
        }

        // Парсим экспоненциальную часть числа
        if (int ch = input.peek(); ch == 'e' || ch == 'E') {
            read_char();
            if (ch = input.peek(); ch == '+' || ch == '-') {
                read_char();
            }
            read_digits();
            is_int = false;
        }

        try {
            if (is_int) {
                // Сначала пробуем преобразовать строку в int
                try {
                    return std::stoi(parsed_num);
                } catch (...) {
                    // В случае неудачи, например, при переполнении,
                    // код ниже попробует преобразовать строку в double
                }
            }
            return std::stod(parsed_num);
        } catch (...) {
            throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
        }
    }

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
    std::string LoadString(std::istream& input) {
        using namespace std::literals;

        auto it = std::istreambuf_iterator<char>(input);
        auto end = std::istreambuf_iterator<char>();
        std::string s;
        while (true) {
            if (it == end) {
                // Поток закончился до того, как встретили закрывающую кавычку?
                throw ParsingError("String parsing error");
            }
            const char ch = *it;
            if (ch == '"') {
                // Встретили закрывающую кавычку
                ++it;
                break;
            } else if (ch == '\\') {
                // Встретили начало escape-последовательности
                ++it;
                if (it == end) {
                    // Поток завершился сразу после символа обратной косой черты
                    throw ParsingError("String parsing error");
                }
                const char escaped_char = *(it);
                // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                }
            } else if (ch == '\n' || ch == '\r') {
                // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                throw ParsingError("Unexpected end of line"s);
            } else {
                // Просто считываем очередной символ и помещаем его в результирующую строку
                s.push_back(ch);
            }
            ++it;
        }

        return s;
    }

    Node LoadNode(istream& input);

    Node LoadArray(istream& input) {
        Array result;

        for (char c; input >> c && c != ']';) {
            if (c != ',') {
                input.putback(c);
            }

            result.push_back(LoadNode(input));

        }

        if (input.eof()) {
            throw ParsingError("Failed to read"s);
        }

        return Node(result);
    }

    Node LoadDict(istream& input) {
        Dict result;

        for (char c; input >> c && c != '}';) {
            if (c == ',') {
                input >> c;
            }
            std::string key = LoadString(input);
            input >> c;
            result.insert({move(key), LoadNode(input)});
        }

        if (input.eof()) {
            throw ParsingError("Failed to read"s);
        }

        return Node(result);
    }

    Node LoadBool(istream& input) {
        char c;
        input >> c;

        static const std::string STRING_TRUE = "true"s;
        static const std::string STRING_FALSE = "false"s;

        if (c == 't') {
            for (int i = 1; i < static_cast<int>(STRING_TRUE.size()); ++i) {

                if (input.eof()) {
                    throw ParsingError("Failed to read bool from stream"s);
                }
                c = input.get();
                if (c != STRING_TRUE[i]) {
                    throw ParsingError("Failed to read bool from stream"s);
                }
            }
            return Node(bool{true});
        }

        if (c == 'f') {
            for (int i = 1; i < static_cast<int>(STRING_FALSE.size()); ++i) {
                if (input.eof()) {
                    throw ParsingError("Failed to read bool from stream"s);
                }
                c = input.get();
                if (c != STRING_FALSE[i]) {
                    throw ParsingError("Failed to read bool from stream"s);
                    break;
                }
            }
            return Node(bool{false});
        }
        return Node{};
    }

    Node LoadNull(istream& input) {
        char c;

        static const std::string STRING_NULL = "null"s;

        for (int i = 0; i < static_cast<int>(STRING_NULL.size()); ++i) {
            input >> c;
            if (input.eof()) {
                throw ParsingError("Failed to read null from stream"s);
            }
            if (c != STRING_NULL[i]) {
                throw ParsingError("Failed to read null from stream"s);
            }
        }

        return Node{};
    }

    Node LoadNode(istream& input) {
        char c;
        input >> c;

        if (input.eof()) {
            throw ParsingError("Failed to read"s);
        }

        switch (c) {
            case '[':
                return LoadArray(input);
            case '{':
                return LoadDict(input);
            case 't':
                input.putback(c);
                return LoadBool(input);
            case 'f':
                input.putback(c);
                return LoadBool(input);
            case 'n':
                input.putback(c);
                return LoadNull(input);
            case '"':
                return Node(LoadString(input));
            default:
                input.putback(c);
                auto tmp = LoadNumber(input);
                if (std::holds_alternative<int>(tmp)) {
                    return Node(std::get<int>(tmp));
                } else {
                    return Node(std::get<double>(tmp));
                }
        }
    }

// ---------------- DOCUMENT LOAD -----------------

    Document::Document(Node root)
            : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{LoadNode(input)};
    }

// ---------------- PRINT FUNCTIONS -----------------

// Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const {
            for (int i = 0; i < indent; ++i) {
                out << "    ";
            }
        }

        // Возвращает новый контекст вывода с увеличенным смещением
        PrintContext Indented() const {
            return {out, indent_step, indent + 1};
        }
    };

    void PrintNode(const Node& node, const PrintContext& ctx);

    void Print(const Document& doc, std::ostream& output) {
        auto root_node = doc.GetRoot();
        PrintNode(root_node, PrintContext{output});
    }

    // Шаблон, подходящий для вывода double и int
    template <typename Value>
    void PrintValue(const Value& value, const PrintContext& ctx) {
        ctx.out << value;
    }

// Перегрузка функции PrintValue для вывода значений null
    void PrintValue(std::nullptr_t, const PrintContext& ctx) {
        ctx.out << "null"sv;
    }

    void PrintValue(const std::string& value, const PrintContext& ctx) {
        ctx.out << "\""sv;

        for (const auto& i : value) {
            switch (i) {
                case '\n':
                    ctx.out << "\\n"sv;
                    break;
                case '\r':
                    ctx.out << "\\r"sv;
                    break;
                case '\\':
                    ctx.out << "\\\\"sv;
                    break;
                case '\"':
                    ctx.out << "\\\""sv;
                    break;
                default:
                    ctx.out << i;
            }
        }
        ctx.out << "\""sv;
    }

// Перегрузка функции PrintValue для вывода значений Dict
    void PrintValue(const Dict& dict, const PrintContext& ctx) {

        ctx.PrintIndent();
        ctx.out << "{"sv << std::endl;
        bool is_first = true;
        for (const auto& [key, value] : dict) {

            if (is_first) {
                is_first = false;

            } else {
                ctx.out << ","sv << std::endl;
            }
            ctx.PrintIndent();
            ctx.PrintIndent();
            ctx.out << "\""sv << key << "\": "sv;
            PrintNode(value, ctx.Indented());
        }
        ctx.out << std::endl;
        ctx.PrintIndent();
        ctx.out << "}"sv;
    }

// Перегрузка функции PrintValue для вывода значений Array
    void PrintValue(const Array& arr, const PrintContext& ctx) {

        ctx.out << "["sv  << std::endl;
        bool is_first = true;

        for (const auto& i : arr) {
            if (is_first) {
                is_first = false;
            } else {
                ctx.out << ","sv << std::endl;
            }
            ctx.PrintIndent();
            ctx.PrintIndent();
            PrintNode(i, ctx.Indented());

        }
        ctx.out << std::endl;
        ctx.PrintIndent();

        ctx.out << "]"sv;
    }

// Перегрузка функции PrintValue для вывода значений Bool
    void PrintValue(bool bool_item, const PrintContext& ctx) {
        if (bool_item) {
            ctx.out << "true"sv;
        } else {
            ctx.out << "false"sv;
        }
    }


    void PrintNode(const Node& node, const PrintContext& ctx) {
        std::visit(
                [&ctx](const auto& value){ PrintValue(value, ctx); },
                node.GetValue());
    }

}  // namespace json