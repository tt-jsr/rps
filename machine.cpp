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

Machine::Machine()
: list_maxcount(20)
, map_maxcount(20)
, debug(true) 
{
}

void Machine::CreateModule(const std::string& name)
{
    Module mod;
    mod.module_name_ = name;
    modules_.emplace(name, std::move(mod));
}

void Machine::push(ObjectPtr& optr)
{
    assert(optr->type != OBJECT_COMMAND);
    stack_.push_back(optr);
}


void Machine::pop(ObjectPtr& optr)
{
    if (stack_.empty())
        throw std::runtime_error("stack underflow");
    optr = stack_.back();
    stack_.pop_back();
}

void Machine::pop(int64_t& v)
{
    if (stack_.empty())
        throw std::runtime_error("stack underflow");
    ObjectPtr& optr = stack_.back();
    if (optr->type == OBJECT_INTEGER)
    {
        Integer *ip = (Integer *)optr.get();
        v = ip->value;
        stack_.pop_back();
        return;
    }
    push(optr);
    std::stringstream strm;
    strm << "pop: Expected integer, got: " << ToStr(*this, optr);
    throw std::runtime_error(strm.str().c_str());
}

void Machine::push(int64_t v)
{
    ObjectPtr optr;
    optr.reset(new Integer(v));
    stack_.push_back(optr);
}

void Machine::pop(ListPtr& lp)
{
    if (stack_.empty())
        throw std::runtime_error("stack underflow");
    ObjectPtr& optr = stack_.back();
    if (optr->type == OBJECT_LIST)
    {
        lp = std::static_pointer_cast<List>(optr);;
        stack_.pop_back();
        return;
    }
    push(optr);
    std::stringstream strm;
    strm << "pop: Expected list, got: " << ToStr(*this, optr);
    throw std::runtime_error(strm.str().c_str());
}

void Machine::push(ListPtr& lp)
{
    ObjectPtr op = lp;
    push(op);
}

void Machine::pop(MapPtr& mp)
{
    if (stack_.empty())
        throw std::runtime_error("stack underflow");
    ObjectPtr& optr = stack_.back();
    if (optr->type == OBJECT_MAP)
    {
        mp = std::static_pointer_cast<Map>(optr);;
        stack_.pop_back();
        return;
    }
    push(optr);
    std::stringstream strm;
    strm << "pop: Expected Map, got: " << ToStr(*this, optr);
    throw std::runtime_error(strm.str().c_str());
}

void Machine::push(MapPtr& mp)
{
    ObjectPtr op = mp;
    push(op);
}

void Machine::pop(std::string& v)
{
    if (stack_.empty())
        throw std::runtime_error("stack underflow");
    ObjectPtr& optr = stack_.back();
    if (optr->type == OBJECT_STRING)
    {
        String *sp = (String *)optr.get();
        v = sp->value;
        stack_.pop_back();
        return;
    }
    push(optr);
    std::stringstream strm;
    strm << "pop: Expected string, got: " << ToStr(*this, optr);
    throw std::runtime_error(strm.str().c_str());
}

void Machine::push(const std::string& v)
{
    ObjectPtr optr;
    optr.reset(new String(v));
    stack_.push_back(optr);
}

ObjectPtr& Machine::peek()
{
    if (stack_.empty())
        throw std::runtime_error("stack underflow");
    return stack_.back();
}

ObjectPtr& Machine::peek(size_t n)
{
    ++n;
    if (stack_.size() < n)
        throw std::runtime_error("stack underflow");
    return stack_[stack_.size()-n];
}

/****************************************************************/
void Execute(Machine& machine, std::vector<ObjectPtr>& vec)
{
    for (ObjectPtr& op : vec)
    {
        EVAL(machine, op);
    }
}

void EVAL(Machine& machine, ObjectPtr optr)
{
    switch(optr->type)
    {
    case OBJECT_STRING:
        machine.stack_.push_back(optr);
        break;
    case OBJECT_INTEGER:
        machine.stack_.push_back(optr);
        break;
    case OBJECT_LIST:
        machine.stack_.push_back(optr);
        break;
    case OBJECT_PROGRAM:
        {
            Program *p = (Program *)optr.get();
            std::string modname = machine.current_module_;
            machine.current_module_ = p->module_name;
            try
            {
                Execute(machine, p->program);
            }
            catch (std::exception& e)
            {
                machine.current_module_ = modname;
                throw;
            }
            machine.current_module_ = modname;
        }
        break;
    case OBJECT_IF:
        {
            ObjectPtr ob;
            If *p = (If *)optr.get();
            Execute(machine, p->cond);
            machine.pop(ob);
            if (ToBool(ob))
                Execute(machine, p->then);
            else
                Execute(machine, p->els);
        }
        break;
    case OBJECT_COMMAND:
        {
            Command *pCommand = (Command *)optr.get();
            (*pCommand->funcptr)(machine);
        }
        break;
    default:
        std::cout << "=== EVAL: " << ToStr(machine, optr) << std::endl;
        assert(false);
        break;
    }
}

void EVAL(Machine& machine)
{
    ObjectPtr optr;
    machine.pop(optr);
    EVAL(machine, optr);
}

void CALL(Machine& machine)
{
    RCL(machine);
    EVAL(machine);
}

void MODULES(Machine& machine)
{
    ListPtr lp = MakeList();
    for (auto& pr : machine.modules_)
    {
        StringPtr sp = MakeString();
        sp->value = pr.first;
        lp->items.push_back(sp);
    }
    machine.push(lp);
}
