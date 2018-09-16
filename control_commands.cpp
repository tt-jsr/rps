#include <vector>
#include <unordered_map>
#include <exception>
#include <iostream>
#include <sstream>
#include "token.h"
#include "object.h"
#include "module.h"
#include "machine.h"
#include "utilities.h"
#include "commands.h"

void IFT(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "Help not available" << std::endl;
        return;
    }

    if (machine.stack_.size() < 2)
        throw std::runtime_error("IFT: stack underflow");
    ObjectPtr cond;
    ObjectPtr then;
    try
    {
        machine.pop(then);
        machine.pop(cond);
        EVAL(machine, cond);
        int64_t result;
        machine.pop(result);
        if (result)
            EVAL(machine, then);
    }
    catch (std::exception& e)
    {
        machine.push(cond);
        machine.push(then);
        throw;
    }
}

void IFTE(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "Help not available" << std::endl;
        return;
    }

    if (machine.stack_.size() < 3)
        throw std::runtime_error("IFTE: stack underflow");
    ObjectPtr cond;
    ObjectPtr then;
    ObjectPtr els;
    try {
        machine.pop(els);
        machine.pop(then);
        machine.pop(cond);
        EVAL(machine, cond);
        int64_t result;
        machine.pop(result);
        if (result)
            EVAL(machine, then);
        else
            EVAL(machine, els);
    }
    catch (std::exception&)
    {
        machine.push(cond);
        machine.push(then);
        machine.push(els);
        throw;
    }
}

void TRYCATCH(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "Help not available" << std::endl;
        return;
    }

    if (machine.stack_.size() < 2)
        throw std::runtime_error("TRYCATCH: stack underflow");
    ObjectPtr try_;
    ObjectPtr catch_;
    machine.pop(try_);
    machine.pop(catch_);
    try
    {
        EVAL(machine, try_);
    }
    catch (std::exception&)
    {
        EVAL(machine, catch_);
    }
}

