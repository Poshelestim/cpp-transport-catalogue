#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json
{

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error
{
public:
    using runtime_error::runtime_error;
};

class Node final :
        private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>
{
public:
    using variant::variant;
    using Value = variant;

    bool IsNull() const;
    bool IsBool() const;
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsString() const;
    bool IsArray() const;
    bool IsDict() const;

    friend bool operator==(const Node& lhs, const Node& rhs)
    {
        return lhs.GetValue() == rhs.GetValue();
    }

    friend bool operator!=(const Node& lhs, const Node& rhs)
    {
        return !(lhs == rhs);
    }

    const Value &GetValue() const;
    bool AsBool() const;
    int AsInt() const;
    double AsDouble() const;
    const std::string &AsString() const;
    const Array &AsArray() const;
    const Dict &AsDict() const;
};

struct PrintContext
{
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const;

    [[nodiscard]] PrintContext Indented() const;
};

struct NodePrinter
{
    std::ostream &out;
    PrintContext &ctx;

    void operator()(std::nullptr_t) const;
    void operator()(bool value) const;
    void operator()(int value) const;
    void operator()(double value) const;
    void operator()(const std::string &str) const;
    void operator()(const Array &array) const;
    void operator()(const Dict &map) const;
};

class Document
{
public:
    explicit Document(Node root);

    [[nodiscard]] const Node& GetRoot() const;

    friend bool operator==(const Document& left, const Document& right)
    {
        return left.root_ == right.root_;
    }

    friend bool operator!=(const Document& left, const Document& right)
    {
        return left.root_ != right.root_;
    }

private:
    Node root_;
};

bool operator==(const Node &lhs, const Node &rhs);

bool operator!=(const Node &lhs, const Node &rhs);

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json
