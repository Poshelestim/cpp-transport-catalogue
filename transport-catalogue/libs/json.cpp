#include "json.h"

using namespace std;

namespace json
{

Node LoadNode(istream& input);

Node LoadArray(istream& input)
{
    Array result;

    for (char c; input >> c && c != ']';)
    {
        if (c != ',')
        {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    if (!input)
    {
        throw ParsingError("Need ']' symbol to close array");
    }

    return Node(std::move(result));
}

Node LoadString(istream& input)
{
    using namespace std::literals;

    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string result;
    while (true)
    {
        if (it == end)
        {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }

        const char ch = *it;
        if (ch == '"')
        {
            // Встретили закрывающую кавычку
            ++it;
            break;
        }

        if (ch == '\\')
        {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end)
            {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char)
            {
            case 'n':
                result.push_back('\n');
                break;
            case 't':
                result.push_back('\t');
                break;
            case 'r':
                result.push_back('\r');
                break;
            case '"':
                result.push_back('"');
                break;
            case '\\':
                result.push_back('\\');
                break;
            default:
                // Встретили неизвестную escape-последовательность
                throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        }
        else if (ch == '\n' || ch == '\r')
        {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        }
        else
        {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            result.push_back(ch);
        }
        ++it;
    }

    return Node(std::move(result));
}

using Number = std::variant<int, double>;

Node LoadNumber(std::istream& input)
{
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input]()
    {
        parsed_num += static_cast<char>(input.get());
        if (!input)
        {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char]()
    {
        if (std::isdigit(input.peek()) == 0)
        {
            throw ParsingError("A digit is expected"s);
        }

        while (std::isdigit(input.peek()) != 0)
        {
            read_char();
        }
    };

    if (input.peek() == '-')
    {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0')
    {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    }
    else
    {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.')
    {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E')
    {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-')
        {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try
    {
        if (is_int)
        {
            // Сначала пробуем преобразовать строку в int
            try
            {
                return Node(std::stoi(parsed_num));
            }
            catch (...)
            {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node(std::stod(parsed_num));
    }
    catch (...)
    {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadDict(istream& input) {
    Dict result;

    for (char c = '\0'; input >> c && c != '}';)
    {
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input).AsString();
        input >> c;
        result.insert({std::move(key), LoadNode(input)});
    }

    if (!input)
    {
        throw ParsingError("Need '}' symbol to close map");
    }

    return Node(std::move(result));
}

Node LoadNull(istream& input)
{
    if ( input.get() == 'n' &&
         input.get() == 'u' &&
         input.get() == 'l' &&
         input.get() == 'l' )
    {
        return Node(nullptr);
    }

    throw json::ParsingError("Fail to read null from stream"s);
}

Node LoadBool(istream& input)
{
    if (bool value; input >> std::boolalpha >> value)
    {
        return Node(value);
    }

    throw ParsingError("Fail to read bool from stream"s);
}

Node LoadNode(istream& input)
{
    char c = 0;
    input >> c;

    if (c == 'n')
    {
        input.putback(c);
        return LoadNull(input);
    }

    if (c == 't' || c == 'f')
    {
        input.putback(c);
        return LoadBool(input);
    }

    if (c == '[')
    {
        return LoadArray(input);
    }

    if (c == '{')
    {
        return LoadDict(input);
    }

    if (c == '"')
    {
        return LoadString(input);
    }

    input.putback(c);
    return LoadNumber(input);
}

bool Node::IsNull() const
{
    return std::holds_alternative<nullptr_t>(*this);
}

bool Node::IsBool() const
{
    return std::holds_alternative<bool>(*this);
}

bool Node::IsInt() const
{
    return std::holds_alternative<int>(*this);
}

bool Node::IsDouble() const
{
    return holds_alternative<double>(*this) ||
            holds_alternative<int>(*this);
}

bool Node::IsPureDouble() const
{
    return holds_alternative<double>(*this);
}

bool Node::IsString() const
{
    return std::holds_alternative<std::string>(*this);
}

bool Node::IsArray() const
{
    return std::holds_alternative<Array>(*this);
}

bool Node::IsDict() const
{
    return std::holds_alternative<Dict>(*this);
}

const Node::Value &Node::GetValue() const
{
    return *this;
}

bool Node::AsBool() const
{
    if (IsBool())
    {
        return std::get<bool>(*this);
    }

    throw std::logic_error("Not a bool");
}

int Node::AsInt() const
{
    if (IsInt())
    {
        return std::get<int>(*this);
    }

    throw std::logic_error("Not an integer");
}

double Node::AsDouble() const
{
    using namespace std::literals;
    if (IsInt())
    {
        return static_cast<double>(std::get<int>(*this));
    }

    if (IsPureDouble())
    {
        return std::get<double>(*this);
    }

    throw std::logic_error("Not a double"s);
}

const string& Node::AsString() const
{
    if (IsString())
    {
        return std::get<string>(*this);
    }

    throw std::logic_error("Not a string");
}

const Array& Node::AsArray() const
{
    if (IsArray())
    {
        return std::get<Array>(*this);
    }

    throw std::logic_error("Not an array");
}

const Dict& Node::AsDict() const
{
    if (IsDict())
    {
        return std::get<Dict>(*this);
    }

    throw std::logic_error("Not a map");
}

void PrintContext::PrintIndent() const
{
    const std::string str_indent(indent, ' ');
    out << str_indent;
}

PrintContext PrintContext::Indented() const
{
    return {out, indent_step, indent_step + indent};
}

void NodePrinter::operator()(std::nullptr_t /*value*/) const
{
    out << "null"s;
}

void NodePrinter::operator()(bool value) const
{
    out << boolalpha << value << noboolalpha;
}

void NodePrinter::operator()(int value) const
{
    out << value;
}

void NodePrinter::operator()(double value) const
{
    out << value;
}

void NodePrinter::operator()(const std::string &str) const
{
    out << R"(")";
    for (const char ch : str)
    {
        switch (ch)
        {
        case '\n':
            out << R"(\n)";
            break;
        case '\r':
            out << R"(\r)";
            break;
        case '"':
            out << R"(\")";
            break;
        case '\\':
            out << R"(\\)";
            break;
        default:
            out << ch;
            break;
        }
    }
    out << R"(")";
}

void NodePrinter::operator()(const Array &array) const
{
    out << '[' << std::endl;

    bool is_first = true;
    auto inner_ctx = ctx.Indented();
    for (const auto &value : array)
    {
        if (!is_first)
        {
            out << ","sv << std::endl;
        }
        is_first = false;
        inner_ctx.PrintIndent();
        std::visit(NodePrinter{out, inner_ctx}, value.GetValue());
    }

    out << std::endl;
    ctx.PrintIndent();
    out << ']';
}

void NodePrinter::operator()(const Dict &map) const
{
    out << '{' << std::endl;

    bool is_first = true;
    auto inner_ctx = ctx.Indented();
    for (const auto &[key, value] : map)
    {
        if (!is_first)
        {
            out << ","sv << std::endl;
        }
        is_first = false;

        inner_ctx.PrintIndent();
        std::visit(NodePrinter{out, inner_ctx}, Node{key}.GetValue());
        out << ": "sv;
        std::visit(NodePrinter{out, inner_ctx}, value.GetValue());
    }
    out << std::endl;
    ctx.PrintIndent();
    out << '}';
}

Document::Document(Node root) :
    root_(std::move(root))
{

}

const Node& Document::GetRoot() const
{
    return root_;
}

Document Load(istream& input)
{
    return Document{LoadNode(input)};
}

void Print(const Document& doc, ostream& output)
{
    PrintContext ctx{output};
    std::visit(NodePrinter{output, ctx}, doc.GetRoot().GetValue());
}

}  // namespace json
