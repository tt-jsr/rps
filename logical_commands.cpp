#include <vector>
#include <unordered_map>
#include <exception>
#include <iostream>
#include <sstream>
#include <memory>
#include <cassert>
#include "token.h"
#include "object.h"
#include "module.h"
#include "machine.h"
#include "commands.h"

enum Operator
{
    OP_EQ
    ,OP_NEQ
    ,OP_LT
    ,OP_LTEQ
    ,OP_GT
    ,OP_GTEQ
    ,OP_AND
    ,OP_OR
};

void LogicalInteger(Machine& machine, ObjectPtr olhs, ObjectPtr orhs, Operator oper)
{
    assert(olhs->type == OBJECT_INTEGER);
    assert(orhs->type == OBJECT_INTEGER);
    IntegerPtr lhs = std::static_pointer_cast<Integer>(olhs);
    IntegerPtr rhs = std::static_pointer_cast<Integer>(orhs);
    switch (oper)
    {
    case OP_EQ:
        machine.push(lhs->value == rhs->value ? 1 : 0);
        break;
    case OP_NEQ:
        machine.push(lhs->value != rhs->value ? 1 : 0);
        break;
    case OP_LT:
        machine.push(lhs->value < rhs->value ? 1 : 0);
        break;
    case OP_LTEQ:
        machine.push(lhs->value <= rhs->value ? 1 : 0);
        break;
    case OP_GT:
        machine.push(lhs->value > rhs->value ? 1 : 0);
        break;
    case OP_GTEQ:
        machine.push(lhs->value >= rhs->value ? 1 : 0);
        break;
    case OP_AND:
        machine.push(lhs->value && rhs->value ? 1 : 0);
        break;
    case OP_OR:
        machine.push(lhs->value || rhs->value ? 1 : 0);
        break;
    default:
        assert(false);
    }
}

void LogicalString(Machine& machine, ObjectPtr olhs, ObjectPtr orhs, Operator oper)
{
    assert(olhs->type == OBJECT_STRING);
    assert(orhs->type == OBJECT_STRING);
    StringPtr lhs = std::static_pointer_cast<String>(olhs);
    StringPtr rhs = std::static_pointer_cast<String>(orhs);
    switch (oper)
    {
    case OP_EQ:
        machine.push(lhs->value == rhs->value ? 1 : 0);
        break;
    case OP_NEQ:
        machine.push(lhs->value != rhs->value ? 1 : 0);
        break;
    case OP_LT:
        machine.push(lhs->value < rhs->value ? 1 : 0);
        break;
    case OP_LTEQ:
        machine.push(lhs->value <= rhs->value ? 1 : 0);
        break;
    case OP_GT:
        machine.push(lhs->value > rhs->value ? 1 : 0);
        break;
    case OP_GTEQ:
        machine.push(lhs->value >= rhs->value ? 1 : 0);
        break;
    case OP_AND:
        machine.push(!lhs->value.empty() && !rhs->value.empty() ? 1 : 0);
        break;
    case OP_OR:
        machine.push(!lhs->value.empty() || !rhs->value.empty() ? 1 : 0);
        break;
    default:
        assert(false);
    }
}

void EQ(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "EQ: Compare for equality";
        machine.helpstrm() << "\"str1\" \"str2\" EQ => int";
        machine.helpstrm() << "int1 int2 EQ => int";
        return;
    }

    ObjectPtr lhs, rhs;
    if (machine.stack_.size() < 2)
        throw std::runtime_error("EQ requires two arguments");
    machine.pop(rhs);
    machine.pop(lhs);
    if (lhs->type != rhs->type)
    {
        machine.push(lhs);
        machine.push(rhs);
        throw std::runtime_error("EQ: Arguments must be the same type");
    }
    if (lhs->type == OBJECT_INTEGER)
        LogicalInteger(machine, lhs, rhs, OP_EQ);
    else if (lhs->type == OBJECT_STRING)
        LogicalString(machine, lhs, rhs, OP_EQ);
    else
        throw std::runtime_error("EQ: Argument must be String or Integer");
}

void NEQ(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "NEQ: Compare for not equal";
        machine.helpstrm() << "\"str1\" \"str2\" NEQ => int";
        machine.helpstrm() << "int1 int2 NEQ => int";
        return;
    }

    ObjectPtr lhs, rhs;
    if (machine.stack_.size() < 2)
        throw std::runtime_error("NEQ requires two arguments");
    machine.pop(rhs);
    machine.pop(lhs);
    if (lhs->type != rhs->type)
    {
        machine.push(lhs);
        machine.push(rhs);
        throw std::runtime_error("NQ: Arguments must be the same type");
    }
    if (lhs->type == OBJECT_INTEGER)
        LogicalInteger(machine, lhs, rhs, OP_NEQ);
    else if (lhs->type == OBJECT_STRING)
        LogicalString(machine, lhs, rhs, OP_NEQ);
    else
        throw std::runtime_error("NEQ: Argument must be String or Integer");
}

void LT(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "LT: Compare for less than";
        machine.helpstrm() << "\"str1\" \"str2\" LT => int";
        machine.helpstrm() << "int1 int2 LT => int";
        return;
    }

    ObjectPtr lhs, rhs;
    if (machine.stack_.size() < 2)
        throw std::runtime_error("LT requires two arguments");
    machine.pop(rhs);
    machine.pop(lhs);
    if (lhs->type != rhs->type)
    {
        machine.push(lhs);
        machine.push(rhs);
        throw std::runtime_error("LT: Arguments must be the same type");
    }
    if (lhs->type == OBJECT_INTEGER)
        LogicalInteger(machine, lhs, rhs, OP_LT);
    else if (lhs->type == OBJECT_STRING)
        LogicalString(machine, lhs, rhs, OP_LT);
    else
        throw std::runtime_error("LT: Argument must be String or Integer");
}

void LTEQ(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "LTEQ: Compare for less than or equal";
        machine.helpstrm() << "\"str1\" \"str2\" LTEQ => int";
        machine.helpstrm() << "int1 int2 LTEQ => int";
        return;
    }

    ObjectPtr lhs, rhs;
    if (machine.stack_.size() < 2)
        throw std::runtime_error("LTEQ requires two arguments");
    machine.pop(rhs);
    machine.pop(lhs);
    if (lhs->type != rhs->type)
    {
        machine.push(lhs);
        machine.push(rhs);
        throw std::runtime_error("LTEQ: Arguments must be the same type");
    }
    if (lhs->type == OBJECT_INTEGER)
        LogicalInteger(machine, lhs, rhs, OP_LTEQ);
    else if (lhs->type == OBJECT_STRING)
        LogicalString(machine, lhs, rhs, OP_LTEQ);
    else
        throw std::runtime_error("LTEQ: Argument must be String or Integer");
}

void GT(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "GT: Compare for greater than";
        machine.helpstrm() << "\"str1\" \"str2\" GT => int";
        machine.helpstrm() << "int1 int2 GT => int";
        return;
    }

    ObjectPtr lhs, rhs;
    if (machine.stack_.size() < 2)
        throw std::runtime_error("GT requires two arguments");
    machine.pop(rhs);
    machine.pop(lhs);
    if (lhs->type != rhs->type)
    {
        machine.push(lhs);
        machine.push(rhs);
        throw std::runtime_error("GT: Arguments must be the same type");
    }
    if (lhs->type == OBJECT_INTEGER)
        LogicalInteger(machine, lhs, rhs, OP_GT);
    else if (lhs->type == OBJECT_STRING)
        LogicalString(machine, lhs, rhs, OP_GT);
    else
        throw std::runtime_error("GT: Argument must be String or Integer");
}

void GTEQ(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "GTEQ: Compare for greater than or equal";
        machine.helpstrm() << "\"str1\" \"str2\" GTEQ => int";
        machine.helpstrm() << "int1 int2 GTEQ => int";
        return;
    }

    ObjectPtr lhs, rhs;
    if (machine.stack_.size() < 2)
        throw std::runtime_error("GTEQ requires two arguments");
    machine.pop(rhs);
    machine.pop(lhs);
    if (lhs->type != rhs->type)
    {
        machine.push(lhs);
        machine.push(rhs);
        throw std::runtime_error("GTEQ: Arguments must be the same type");
    }
    if (lhs->type == OBJECT_INTEGER)
        LogicalInteger(machine, lhs, rhs, OP_GTEQ);
    else if (lhs->type == OBJECT_STRING)
        LogicalString(machine, lhs, rhs, OP_GTEQ);
    else
        throw std::runtime_error("GTEQ: Argument must be String or Integer");
}

void AND(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "AND: Logical AND";
        machine.helpstrm() << "\"str1\" \"str2\" AND => int";
        machine.helpstrm() << "int1 int2 AND => int";
        return;
    }

    ObjectPtr lhs, rhs;
    if (machine.stack_.size() < 2)
        throw std::runtime_error("AND requires two arguments");
    machine.pop(rhs);
    machine.pop(lhs);
    if (lhs->type != rhs->type)
    {
        machine.push(lhs);
        machine.push(rhs);
        throw std::runtime_error("AND: Arguments must be the same type");
    }
    if (lhs->type == OBJECT_INTEGER)
        LogicalInteger(machine, lhs, rhs, OP_AND);
    else if (lhs->type == OBJECT_STRING)
        LogicalString(machine, lhs, rhs, OP_AND);
    else
        throw std::runtime_error("AND: Argument must be String or Integer");
}

void OR(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "OR: Logical OR";
        machine.helpstrm() << "\"str1\" \"str2\" OR => int";
        machine.helpstrm() << "int1 int2 OR => int";
        return;
    }

    ObjectPtr lhs, rhs;
    if (machine.stack_.size() < 2)
        throw std::runtime_error("OR requires two arguments");
    machine.pop(rhs);
    machine.pop(lhs);
    if (lhs->type != rhs->type)
    {
        machine.push(lhs);
        machine.push(rhs);
        throw std::runtime_error("OR: Arguments must be the same type");
    }
    if (lhs->type == OBJECT_INTEGER)
        LogicalInteger(machine, lhs, rhs, OP_OR);
    else if (lhs->type == OBJECT_STRING)
        LogicalString(machine, lhs, rhs, OP_OR);
    else
        throw std::runtime_error("OR: Argument must be String or Integer");
}


