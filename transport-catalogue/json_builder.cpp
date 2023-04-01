#include "json_builder.h"

namespace json
{

Context::Context(Builder &_builder) :
    builder_(_builder)
{

}

ValueContext::ValueContext(Builder &_builder) :
    Context(_builder)
{

}

Builder &ValueContext::Value(const Node &value)
{
    return builder_.Value(value);
}

DictContext ValueContext::StartDict()
{
    return builder_.StartDict();
}

ArrayContext ValueContext::StartArray()
{
    return builder_.StartArray();
}

DictValueContext::DictValueContext(Builder &_builder) :
    ValueContext(_builder)
{

}

DictContext DictValueContext::Value(const Node &value)
{
    builder_.Value(value);
    return DictContext(builder_);
}

KeyContext::KeyContext(Builder &_builder) :
    Context(_builder)
{

}

DictValueContext KeyContext::Key(const std::string &key)
{
    return builder_.Key(key);
}

DictContext::DictContext(Builder &_builder) :
    Context(_builder)
{

}

DictValueContext DictContext::Key(const std::string &key)
{
    return builder_.Key(key);
}

Builder &DictContext::EndDict()
{
    return builder_.EndDict();
}

ArrayValueContext::ArrayValueContext(Builder &_builder) :
    ValueContext(_builder)
{

}

ArrayContext ArrayValueContext::Value(const Node &value)
{
    builder_.Value(value);
    return ArrayContext(builder_);
}

ArrayContext::ArrayContext(Builder &_builder) :
    Context(_builder)
{

}

ArrayContext ArrayContext::StartArray()
{
    return builder_.StartArray();
}

Builder &ArrayContext::EndArray()
{
    return builder_.EndArray();
}

DictContext ArrayContext::StartDict()
{
    return builder_.StartDict();
}

ArrayContext &ArrayContext::Value(const Node &value)
{
    builder_.Value(value);
    return *this;
}

Builder::Builder()
{
    nodes_stack_.emplace_back(&root_);
}

Builder::Builder(Builder &&_other) noexcept
{
    root_ = nullptr;
    nodes_stack_.clear();

    std::swap(this->root_, _other.root_);
    std::swap(this->nodes_stack_, _other.nodes_stack_);
}

Builder &Builder::operator=(Builder &&_other) noexcept
{
    root_ = nullptr;
    nodes_stack_.clear();

    std::swap(this->root_, _other.root_);
    std::swap(this->nodes_stack_, _other.nodes_stack_);

    return *this;
}

DictContext Builder::StartDict()
{
    if (nodes_stack_.empty())
    {
        throw std::logic_error("StartDict(): stack is empty");
    }

    if (nodes_stack_.back()->IsArray())
    {
        const_cast<Array &>(nodes_stack_.back()->AsArray()).emplace_back(Dict());
        Node* node = &const_cast<Array&>(nodes_stack_.back()->AsArray()).back();
        nodes_stack_.emplace_back(node);
    }
    else if (nodes_stack_.back()->IsNull())
    {
        *nodes_stack_.back() = Dict();
    }
    else
    {
        throw std::logic_error("StartDict(): Last element in stack is not array or nullptr_t");
    }

    return DictContext(*this);
}

Builder &Builder::EndDict()
{
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict())
    {
        throw std::logic_error("EndDict(): Last element in stack is not map");
    }

    nodes_stack_.pop_back();
    return *this;
}

ArrayContext Builder::StartArray()
{
    if (nodes_stack_.empty())
    {
        throw std::logic_error("StartDict(): stack is empty");
    }

    if (nodes_stack_.back()->IsArray())
    {
        const_cast<Array &>(nodes_stack_.back()->AsArray()).emplace_back(Array());
        Node* node = &const_cast<Array&>(nodes_stack_.back()->AsArray()).back();
        nodes_stack_.emplace_back(node);
    }
    else if (nodes_stack_.back()->IsNull())
    {
        *nodes_stack_.back() = Array();
    }
    else
    {
        throw std::logic_error("StartArray(): Last element in stack is not array or nullptr_t");
    }

    return ArrayContext(*this);
}

Builder &Builder::EndArray()
{
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray())
    {
        throw std::logic_error("EndArray(): Last element in stack is not array");
    }

    nodes_stack_.pop_back();
    return *this;
}

DictValueContext Builder::Key(const std::string &key)
{
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict())
    {
        throw std::logic_error("Key(): Last element in stack is not map");
    }

    Node* node = &const_cast<Dict&>(nodes_stack_.back()->AsDict())[key];
    nodes_stack_.emplace_back(node);
    return DictValueContext(*this);
}

Builder &Builder::Value(const Node &value)
{
    if (nodes_stack_.empty())
    {
        throw std::logic_error("Value(): stack is empty");
    }

    if (nodes_stack_.back()->IsArray())
    {
        const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(value);
    }
    else if (nodes_stack_.back()->IsNull())
    {
        *nodes_stack_.back() = value;
        nodes_stack_.pop_back();
    }
    else
    {
        throw std::logic_error("Value(): Last element in stack is not array or nullptr_t");
    }

    return *this;
}

Node &Builder::Build()
{
    if (!nodes_stack_.empty())
    {
        throw std::logic_error("Build(): stack is not empty");
    }
    return root_;
}

} //namespace json
