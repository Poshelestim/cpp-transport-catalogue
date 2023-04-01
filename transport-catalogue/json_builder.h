#ifndef JSONBUILDER_H
#define JSONBUILDER_H

#include <memory>

#include "libs/json.h"

namespace json
{

class Builder;
class ArrayContext;
class DictContext;

class Context
{
public:
    explicit Context(Builder &_builder);

    Context(const Context &) = default;
    Context(Context &&) = delete;
    Context &operator=(const Context &) = delete;
    Context &operator=(Context &&) = delete;

    virtual ~Context() = default;

protected:
    Builder &builder_;
};

class ValueContext : public Context
{
public:
    explicit ValueContext(Builder &_builder);

    ValueContext(const ValueContext &) = default;
    ValueContext(ValueContext &&) = delete;
    ValueContext &operator=(const ValueContext &) = delete;
    ValueContext &operator=(ValueContext &&) = delete;

    ~ValueContext() override = default;

    Builder &Value(const Node &value);

    DictContext StartDict();

    ArrayContext StartArray();
};

class DictValueContext : public ValueContext
{
public:
    explicit DictValueContext(Builder &_builder);

    DictValueContext(const DictValueContext &) = default;
    DictValueContext(DictValueContext &&) = delete;
    DictValueContext &operator=(const DictValueContext &) = delete;
    DictValueContext &operator=(DictValueContext &&) = delete;

    ~DictValueContext() override = default;

    DictContext Value(const Node &value);
};

class KeyContext : public Context
{
public:
    explicit KeyContext(Builder &_builder);

    KeyContext(const KeyContext &) = default;
    KeyContext(KeyContext &&) = delete;
    KeyContext &operator=(const KeyContext &) = delete;
    KeyContext &operator=(KeyContext &&) = delete;

    ~KeyContext() override = default;

    DictValueContext Key(const std::string &key);
};

class DictContext : public Context
{
public:
    explicit DictContext(Builder &_builder);

    DictContext(const DictContext &) = default;
    DictContext(DictContext &&) = delete;
    DictContext &operator=(const DictContext &) = delete;
    DictContext &operator=(DictContext &&) = delete;

    ~DictContext() override = default;

    DictValueContext Key(const std::string &key);

    Builder &EndDict();
};

class ArrayValueContext : private ValueContext
{
public:
    explicit ArrayValueContext(Builder &_builder);

    ArrayValueContext(const ArrayValueContext &) = default;
    ArrayValueContext(ArrayValueContext &&) = delete;
    ArrayValueContext &operator=(const ArrayValueContext &) = delete;
    ArrayValueContext &operator=(ArrayValueContext &&) = delete;

    ~ArrayValueContext() override = default;

    ArrayContext Value(const Node &value);
};

class ArrayContext : public Context
{
public:
    explicit ArrayContext(Builder &_builder);

    ArrayContext(const ArrayContext &) = default;
    ArrayContext(ArrayContext &&) = delete;
    ArrayContext &operator=(const ArrayContext &) = delete;
    ArrayContext &operator=(ArrayContext &&) = delete;

    ~ArrayContext() override = default;

    ArrayContext StartArray();

    Builder &EndArray();

    DictContext StartDict();

    ArrayContext &Value(const Node &value);
};

class Builder
{
public:
    Builder();

    Builder(const Builder &) = delete;

    Builder(Builder &&_other) noexcept;

    Builder &operator=(const Builder &) = delete;

    Builder &operator=(Builder &&_other) noexcept;

    ~Builder() = default;

    DictContext StartDict();

    Builder &EndDict();

    ArrayContext StartArray();

    Builder &EndArray();

    DictValueContext Key(const std::string& key);

    Builder &Value(const Node& value);

    Node &Build();

private:
    Node root_ = nullptr;
    std::vector<Node *> nodes_stack_;
};

} //namespace json

#endif // JSONBUILDER_H
