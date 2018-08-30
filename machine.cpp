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

Machine::Machine()
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
    switch(optr->type)
    {
    case OBJECT_STRING:
        stack_.push_back(optr);
        break;
    case OBJECT_INTEGER:
        stack_.push_back(optr);
        break;
    case OBJECT_LIST:
        stack_.push_back(optr);
        break;
    case OBJECT_PROGRAM:
        stack_.push_back(optr);
        break;
    case OBJECT_COMMAND:
        {
            try
            {
                Command *pCommand = (Command *)optr.get();
                (*pCommand->funcptr)(*this);
            }
            catch (std::exception& e)
            {
                std::cout << "Exception: " << e.what() << std::endl;
            }
        }
    }
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
    throw std::runtime_error("Not an integer");
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
    throw std::runtime_error("Not a list");
}

void Machine::push(ListPtr& lp)
{
    ObjectPtr op = lp;
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
    throw std::runtime_error("Not a string");
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
            for (ObjectPtr& op : p->program)
            {
                machine.push(op);
            }
        }
        break;
    case OBJECT_COMMAND:
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

