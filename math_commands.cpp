#include <vector>
#include <unordered_map>
#include <exception>
#include <iostream>
#include <sstream>
#include "token.h"
#include "object.h"
#include "module.h"
#include "machine.h"

void ADD(Machine& machine)
{
    int64_t arg1, arg2;
    if (machine.stack_.size() < 2)
        throw std::runtime_error("ADD requires two arguments");
    throw_required(machine, "ADD", 0, OBJECT_INTEGER);
    throw_required(machine, "ADD", 1, OBJECT_INTEGER);
    machine.pop(arg1);
    machine.pop(arg2);
    machine.push(arg1 + arg2);
}

void SUB(Machine& machine)
{
    int64_t arg1, arg2;
    if (machine.stack_.size() < 2)
        throw std::runtime_error("SUB requires two arguments");
    throw_required(machine, "SUB", 0, OBJECT_INTEGER);
    throw_required(machine, "SUB", 1, OBJECT_INTEGER);
    machine.pop(arg1);
    machine.pop(arg2);
    machine.push(arg2 - arg1);
}

void MUL(Machine& machine)
{
    int64_t arg1, arg2;
    if (machine.stack_.size() < 2)
        throw std::runtime_error("MUL requires two arguments");
    throw_required(machine, "MUL", 0, OBJECT_INTEGER);
    throw_required(machine, "MUL", 1, OBJECT_INTEGER);
    machine.pop(arg1);
    machine.pop(arg2);
    machine.push(arg1 * arg2);
}

void DIV(Machine& machine)
{
    int64_t arg1, arg2;
    if (machine.stack_.size() < 2)
        throw std::runtime_error("DIV requires two arguments");
    throw_required(machine, "DIV", 0, OBJECT_INTEGER);
    throw_required(machine, "DIV", 1, OBJECT_INTEGER);
    machine.pop(arg1);
    machine.pop(arg2);
    machine.push(arg2 / arg1);
}


