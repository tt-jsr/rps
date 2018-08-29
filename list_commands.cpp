#include <vector>
#include <unordered_map>
#include <exception>
#include <iostream>
#include <sstream>
#include <cassert>
#include <memory>
#include "token.h"
#include "object.h"
#include "module.h"
#include "machine.h"
#include "commands.h"
#include "utilities.h"


void APPEND(Machine& machine)
{
    ListPtr lp;
    ObjectPtr optr;
    if (machine.stack_.size() < 2)
        throw std::runtime_error("APPEND requires two arguments");
    if (machine.peek(1)->type != OBJECT_LIST)
        throw std::runtime_error("APPEND: requires List argument");
    machine.pop(optr);
    machine.pop(lp);
    if (!lp.unique())
        lp = std::static_pointer_cast<List>(Clone(lp));
    lp->items.push_back(optr);
    machine.push(lp);
}

void GET(Machine& machine)
{
    ListPtr lp;
    int64_t idx;
    if (machine.stack_.size() < 2)
        throw std::runtime_error("GET requires two arguments");
    if (machine.peek(0)->type != OBJECT_INTEGER)
        throw std::runtime_error("GET: requires Integer argument");
    if (machine.peek(1)->type != OBJECT_LIST)
        throw std::runtime_error("GET: requires List argument");
    machine.pop(idx);
    machine.pop(lp);
    if (idx < 0)
    {
        idx = lp->items.size() + idx;
    }
    if (idx < 0 || idx >= lp->items.size())
    {
        machine.push(lp);
        machine.push(idx);
        throw std::runtime_error("GET: Index out of range");
    }
    ObjectPtr p = lp->items[idx];
    machine.push(p);
}

void INSERT(Machine& machine)
{
    ListPtr lp;
    int64_t idx;
    ObjectPtr op;
    if (machine.stack_.size() < 3)
        throw std::runtime_error("INSERT requires three arguments");
    if (machine.peek(1)->type != OBJECT_INTEGER)
        throw std::runtime_error("INSERT: requires Integer argument");
    if (machine.peek(2)->type != OBJECT_LIST)
        throw std::runtime_error("INSERT: requires List argument");
    machine.pop(op);
    machine.pop(idx);
    machine.pop(lp);
    if (idx < 0)
    {
        idx = lp->items.size() + idx;
    }
    if (idx >= lp->items.size())
    {
        machine.push(lp);
        machine.push(idx);
        machine.push(op);
        throw std::runtime_error("INSERT: Index out of range");
    }
    if (!lp.unique())
        lp = std::static_pointer_cast<List>(Clone(lp));
    lp->items.insert(lp->items.begin()+idx, op);
    machine.push(lp);
}

void ERASE(Machine& machine)
{
    ListPtr lp;
    int64_t idx;
    if (machine.stack_.size() < 2)
        throw std::runtime_error("ERASE requires two arguments");
    if (machine.peek(0)->type != OBJECT_INTEGER)
        throw std::runtime_error("ERASE: requires Integer argument");
    if (machine.peek(1)->type != OBJECT_LIST)
        throw std::runtime_error("ERASE: requires List argument");
    machine.pop(idx);
    machine.pop(lp);
    if (idx < 0)
    {
        idx = lp->items.size() + idx;
    }
    if (idx < 0 || idx >= lp->items.size())
    {
        machine.push(lp);
        machine.push(idx);
        throw std::runtime_error("ERASE: Index out of range");
    }
    if (!lp.unique())
        lp = std::static_pointer_cast<List>(Clone(lp));
    lp->items.erase(lp->items.begin() + idx);
    machine.push(lp);
}

void CLEAR(Machine& machine)
{
    ListPtr lp;
    if (machine.stack_.size() < 1)
        throw std::runtime_error("CLEAR requires a List argument");
    if (machine.peek(0)->type != OBJECT_LIST)
        throw std::runtime_error("CLEAR: requires List argument");
    machine.pop(lp);
    if (!lp.unique())
        lp = std::static_pointer_cast<List>(Clone(lp));
    lp->items.clear();
    machine.push(lp);
}

void SIZE(Machine& machine)
{
    ListPtr lp;
    if (machine.stack_.size() < 1)
        throw std::runtime_error("SIZE requires a List argument");
    if (machine.peek(0)->type != OBJECT_LIST)
        throw std::runtime_error("SIZE: requires List argument");
    machine.pop(lp);
    int64_t sz = lp->items.size();
    machine.push(sz);
}




