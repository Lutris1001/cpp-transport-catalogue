#pragma once

#include <cstdint>
#include <iostream>
#include <iomanip>
#include <memory>
#include <string>
#include <deque>
#include <optional>
#include <variant>

namespace svg {
 
enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap);

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};
    
std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join);

// ------------------ RGB ------------------

struct Rgb {
    
    Rgb() 
        : red(0), green(0), blue(0)
    {
    }   
    
    
    Rgb(uint8_t red_color, uint8_t green_color, uint8_t blue_color) 
        : red(red_color), green(green_color), blue(blue_color)
    {
    }   
    
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    
};

// ------------------ RGBA ------------------
    
struct Rgba {
    
    Rgba()
        : red(0), green(0), blue(0), opacity(1.0)
    {
    } 
    
    Rgba(uint8_t red_color, uint8_t green_color, uint8_t blue_color, double alpha) 
        : red(red_color), green(green_color), blue(blue_color), opacity(alpha)
    {
    }
    
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    double opacity = 1.0;
    
};

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
    
    // Объявив в заголовочном файле константу со спецификатором inline,
    // мы сделаем так, что она будет одной на все единицы трансляции,
    // которые подключают этот заголовок.
    // В противном случае каждая единица трансляции будет использовать свою копию этой константы
inline const Color NoneColor{"none"};
    
std::ostream& operator<<(std::ostream& out, const Color& color);

// ------------------ ColorPrinter ------------------
    
struct ColorPrinter {
    
    std::ostream& out;
    
    void operator()(std::monostate) const {
        out << NoneColor;
    }
    void operator()(std::string color) const {
        out << color;
    }
    void operator()(Rgb rgb_color) const {
        using namespace std::literals;
        out << "rgb("sv << std::to_string(rgb_color.red) << ","sv << std::to_string(rgb_color.green) << ","sv << std::to_string(rgb_color.blue) << ")"sv;
    }
    void operator()(Rgba rgba_color) const {
        using namespace std::literals;
        out << "rgba("sv << std::to_string(rgba_color.red) << ","sv << std::to_string(rgba_color.green) << ","sv << std::to_string(rgba_color.blue) << ","sv << rgba_color.opacity << ")"sv;
    }
};

// ------------------ PathProps ------------------
    
template <typename Owner>
class PathProps {
public:
    Owner& SetFillColor(Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeWidth(double width) {
        stroke_width_ = std::move(width);
        return AsOwner();
    }
    Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
        line_cap_ = std::move(line_cap);
        return AsOwner();
    }
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
        line_join_ = std::move(line_join);
        return AsOwner();
    }

protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;

        if (fill_color_) {
            out << " fill=\""sv << *fill_color_ << "\""sv;
        }
        if (stroke_color_) {
            out << " stroke=\""sv << *stroke_color_ << "\""sv; // !!!!!!!!!!!!!
        }
        if (stroke_width_) {
            out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
        }
        if (line_cap_) {
            out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
        }
        if (line_join_) {
            out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
        }
    }

private:
    Owner& AsOwner() {
        // static_cast безопасно преобразует *this к Owner&,
        // если класс Owner — наследник PathProps
        return static_cast<Owner&>(*this);
    }

    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> line_cap_;
    std::optional<StrokeLineJoin> line_join_;
};

// ------------------ Point ------------------

struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;

    bool operator==(const Point& other) {
        return x == other.x && y == other.y;
    }
};

// ------------------ RenderContext ------------------

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

// ------------------ Object ------------------

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */

class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

// ------------------ ObjectContainer ------------------
    
class ObjectContainer {
public:
    
    template <typename Obj>
    void Add(Obj obj);
        
    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    
    virtual ~ObjectContainer() = default;
    
};

// ------------------ Drawable ------------------
    
class Drawable {
public:
    
    virtual void Draw(ObjectContainer& container) const = 0;
    
    virtual ~Drawable() = default;
};

/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object , public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public PathProps<Polyline> {
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

    /*
     * Прочие методы и данные, необходимые для реализации элемента <polyline>
     */
    
private:

    void RenderObject(const RenderContext& context) const override;
    
    std::deque<Point> points_;
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text : public Object, public PathProps<Text> {
public:
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

    // Прочие данные и методы, необходимые для реализации элемента <text>
private:
    Point pos_ = Point(0, 0);
    Point offset_ = Point(0, 0);
    std::string data_;
    std::string font_weight_;
    std::string font_family_;
    uint32_t font_size_ = 1;
    
    void RenderObject(const RenderContext& context) const override;
    
};

// ------------------ Document ------------------

class Document final : public ObjectContainer {
public:

    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;

    // Прочие методы и данные, необходимые для реализации класса Document
    
private:
    std::deque<std::unique_ptr<Object>> objects_;
};

template <typename Obj>
void ObjectContainer::Add(Obj obj) {

    AddPtr(std::make_unique<Obj>(std::move(obj)));
}

}  // namespace svg



