#include "Expression.hpp"
#include "Parser.hpp"

namespace pdal
{
namespace expr
{

Node::Node(NodeType type) : m_type(type)
{}

Node::~Node()
{}

NodeType Node::type() const
{ return m_type; }


UnNode::UnNode(NodeType type, NodePtr sub) :
    Node(type), m_sub(std::move(sub))
{}

std::string UnNode::print() const
{
    return "!(" + m_sub->print() + ")";
}

Utils::StatusWithReason UnNode::prepare(PointLayoutPtr l)
{
    return m_sub->prepare(l);
}
    

BinNode::BinNode(NodeType type, NodePtr left, NodePtr right) :
    Node(type), m_left(std::move(left)), m_right(std::move(right))
{}

std::string BinNode::print() const
{
    std::string s;

    switch (type())
    {
    case NodeType::Add:
        s = "+";
        break;
    case NodeType::Subtract:
        s = "-";
        break;
    case NodeType::Multiply:
        s = "*";
        break;
    case NodeType::Divide:
        s = "/";
        break;
    default:
        break;
    }

    return "(" + m_left->print() + " " + s + " " + m_right->print() + ")";
}

Utils::StatusWithReason BinNode::prepare(PointLayoutPtr l)
{
    auto status = m_left->prepare(l);
    if (status)
        status = m_right->prepare(l);
    return status;
}

/**
virtual double eval(PointRef& p) const
{
double l = m_left->eval(p);
double r = m_right->eval(p);
switch (type())
{
case NodeType::Add:
return l + r;
case NodeType::Subtract:
return l - r;
case NodeType::Multiply:
return l * r;
case NodeType::Divide:
return l / r;
default:
return 0;
}
}
**/

BoolNode::BoolNode(NodeType type, NodePtr left, NodePtr right) :
    Node(type), m_left(std::move(left)), m_right(std::move(right))
{}

std::string BoolNode::print() const
{
    std::string s;
    switch (type())
    {
    case NodeType::And:
        s = "&&";
        break;
    case NodeType::Or:
        s = "||";
        break;
    case NodeType::Equal:
        s = "==";
        break;
    case NodeType::NotEqual:
        s = "!=";
        break;
    case NodeType::Greater:
        s = ">";
        break;
    case NodeType::GreaterEqual:
        s = ">=";
        break;
    case NodeType::Less:
        s = "<";
        break;
    case NodeType::LessEqual:
        s = "<=";
        break;
    default:
        break;
    }

    return "(" + m_left->print() + " " + s + " " + m_right->print() + ")";
}

Utils::StatusWithReason BoolNode::prepare(PointLayoutPtr l)
{
    auto status = m_left->prepare(l);
    if (status)
        status = m_right->prepare(l);
    return status;
}

    /**
    virtual double eval(PointRef& p) const
    {
        switch (type())
        {
        case NodeType::And:
            return m_left->eval(p) && m_right->eval(p);
        case NodeType::Or:
            return m_left->eval(p) || m_right->eval(p);
        default:
            return 0;
        }
    }
    **/

ValNode::ValNode(double d) : Node(NodeType::Value), m_val(d)
{}

std::string ValNode::print() const
{
    return std::to_string(m_val);
}

Utils::StatusWithReason ValNode::prepare(PointLayoutPtr l)
{ return true; }

    /**
    virtual double eval(PointRef&) const
    { return m_val; }
    **/

double ValNode::value() const
{ return m_val; }

VarNode::VarNode(const std::string& s) : Node(NodeType::Identifier), m_name(s),
    m_id(Dimension::Id::Unknown)
{}

std::string VarNode::print() const
{ return m_name; }

Utils::StatusWithReason VarNode::prepare(PointLayoutPtr l)
{
    m_id = l->findDim(m_name);
    if (m_id == Dimension::Id::Unknown)
        return { -1, "Unknown dimension '" + m_name + "' in assigment." };
    return true;
}

/**
    virtual double eval(PointRef& p) const
    { return p.getFieldAs<double>(m_id); }
**/

Expression::Expression()
{}

// This is a strange copy ctor that ignores the source.  At this point we
// don't need it to do anything, but we do need the an expression to
// be copyable.  In order to copy, we'd actually have to deep-copy the
// nodes, but there is no way to do that right now.
Expression::Expression(const Expression& expr)
{}

Expression::~Expression()
{}

// This is a strange copy ctor that ignores the source.  At this point we
// don't need it to do anything, but we do need the an expression to
// be copyable.  In order to copy, we'd actually have to deep-copy the
// nodes, but there is no way to do that right now.
Expression& Expression::operator=(const Expression& expr)
{
    clear();
    return *this;
}

void Expression::clear()
{
    std::stack<NodePtr> empty;
    m_nodes.swap(empty);
    m_error.clear();
}

bool Expression::parse(const std::string& s)
{
    clear();
    Parser p(*this);
    bool ok = p.parse(s);
    if (!ok)
        m_error = p.error();
    return ok;
}

std::string Expression::error() const
{
    return m_error;
}

std::string Expression::print() const
{
    return m_nodes.top()->print();
}

NodePtr Expression::popNode()
{
    NodePtr n(std::move(m_nodes.top()));
    m_nodes.pop();
    return n;
}

void Expression::pushNode(NodePtr node)
{
    m_nodes.push(std::move(node));
}

Utils::StatusWithReason Expression::prepare(PointLayoutPtr layout)
{
    if (m_nodes.size())
        return m_nodes.top()->prepare(layout);
    return true;
}

std::ostream& operator<<(std::ostream& out, const Expression& expr)
{
    out << expr.print();
    return out;
}

} // namespace expr
} // namespace pdal

