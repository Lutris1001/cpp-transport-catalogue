#include "svg.h"

#include <memory>

namespace svg {

using namespace std::literals;
    
std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap) {
    switch (line_cap) {
        case StrokeLineCap::BUTT :
            out << "butt"sv;
            break;
        case StrokeLineCap::ROUND :
            out << "round"sv;
            break;
        case StrokeLineCap::SQUARE :
            out << "square"sv;
            break;
    }
    return out;
}
    
std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join) {
    switch (line_join) {
        case StrokeLineJoin::ARCS :
            out << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL :
            out << "bevel"sv;
            break;
        case StrokeLineJoin::MITER :
            out << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP :
            out << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND :
            out << "round"sv;
            break;
    }
    
    return out;
}
    
std::ostream& operator<<(std::ostream& out, const Color& color) {
    visit(ColorPrinter{out}, color);
    return out;
}

// ------------------ Object ------------------

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ------------------ Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}
    
// ------------------ Polyline ------------------
    
// Добавляет очередную вершину к ломаной линии
Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    bool is_first = true;
    for (auto i : points_) {
        if (is_first) {
            out << i.x << ","sv << i.y;
            is_first = false;
        } else {
            out << " "sv << i.x << ","sv << i.y;
        }
    }
    out << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}
    
// ------------------ Text ------------------

    // Задаёт координаты опорной точки (атрибуты x и y)
Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

    // Задаёт размеры шрифта (атрибут font-size)
Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}

    // Задаёт название шрифта (атрибут font-family)
Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}

    // Задаёт толщину шрифта (атрибут font-weight)
Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
Text& Text::SetData(std::string data) {
    data_ = data;
    return *this;
}

    // Прочие данные и методы, необходимые для реализации элемента <text>
void Text::RenderObject(const RenderContext& context) const {

    auto& out = context.out;
    
    std::string result;
    std::string_view text = std::string_view(data_);
    static const std::string_view signs = "\"<>'&"sv;

    int64_t pos = 0;

    const int64_t pos_end_ = text.npos;
    
    while (!text.empty()) {
        int64_t space = text.find_first_of(signs);
        
        if (space == pos_end_) {
            std::string_view two = (text.substr(pos));
            result += std::string(two);
        } else {
            std::string_view one = (text.substr(pos, space));
            result += std::string(one);
            if (text.at(space) == '"') {
                result += "&quot;"s;
            }
            if (text.at(space) == '\'') {
                result += "&apos;"s;
            }
            if (text.at(space) == '<') {
                result += "&lt;"s;
            }
            if (text.at(space) == '>') {
                result += "&gt;"s;
            }
            if (text.at(space) == '&') {
                result += "&amp;"s;
            }
        }
        text.remove_prefix(std::min(text.find_first_not_of(signs, space), text.size()));
    }

// ------------------

    out << "<text"s;
    
    RenderAttrs(context.out);
    
    out << " x=\""sv << pos_.x << "\""sv; 

    out << " y=\""sv << pos_.y << "\""sv; 

    out << " dx=\""sv << offset_.x << "\""sv; 

    out << " dy=\""sv << offset_.y << "\""sv; 

    out << " font-size=\""sv << font_size_ << "\""sv; 


    if (!font_family_.empty()) {
        out << " font-family=\""sv << font_family_ << "\""sv; 
    }
    
    if (!font_weight_.empty()) {
        out << " font-weight=\""sv << font_weight_ << "\""sv; 
    }

    out << ">"sv << result << "</text>"sv;

}
    
// ------------------ Document -------------------
    
    // Добавляет в svg-документ объект-наследник svg::Object
void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.push_back(std::move(obj));
}

    // Выводит в ostream svg-представление документа
void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl <<
    "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    for (const auto& i : objects_) {
        out << "  "sv;
        i->Render(RenderContext(out));
    }
    out << "</svg>"sv;
}
    
}  // namespace svg

