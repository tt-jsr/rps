#include <vector>
#include <unordered_map>
#include <exception>
#include <iostream>
#include <sstream>
#include "token.h"
#include "object.h"
#include "module.h"
#include "machine.h"

// int int => int
void ADD(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "ADD: Addition" << std::endl;
        std::cout << "int1 int2 ADD => int" << std::endl;
        return;
    }

    int64_t arg1, arg2;
    stack_required(machine, "ADD", 2);
    throw_required(machine, "ADD", 0, OBJECT_INTEGER);
    throw_required(machine, "ADD", 1, OBJECT_INTEGER);
    machine.pop(arg1);
    machine.pop(arg2);
    machine.push(arg1 + arg2);
}

// int int => int
void SUB(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "SUB: Subraction" << std::endl;
        std::cout << "int1 int2 SUB => int" << std::endl;
        return;
    }

    int64_t arg1, arg2;
    stack_required(machine, "SUB", 2);
    throw_required(machine, "SUB", 0, OBJECT_INTEGER);
    throw_required(machine, "SUB", 1, OBJECT_INTEGER);
    machine.pop(arg1);
    machine.pop(arg2);
    machine.push(arg2 - arg1);
}

// int int => int
void MUL(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "MUL: Multilication" << std::endl;
        std::cout << "int1 int2 MUL => int" << std::endl;
        return;
    }

    int64_t arg1, arg2;
    stack_required(machine, "MUL", 2);
    throw_required(machine, "MUL", 0, OBJECT_INTEGER);
    throw_required(machine, "MUL", 1, OBJECT_INTEGER);
    machine.pop(arg1);
    machine.pop(arg2);
    machine.push(arg1 * arg2);
}

// int int => int
void DIV(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "DIV: Division" << std::endl;
        std::cout << "int1 int2 DIV => int" << std::endl;
        return;
    }

    int64_t arg1, arg2;
    stack_required(machine, "DIV", 2);
    throw_required(machine, "DIV", 0, OBJECT_INTEGER);
    throw_required(machine, "DIV", 1, OBJECT_INTEGER);
    machine.pop(arg1);
    machine.pop(arg2);
    machine.push(arg2 / arg1);
}

// int => int
void INC(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "INC: Increment" << std::endl;
        std::cout << "int INC => int" << std::endl;
        return;
    }

    stack_required(machine, "INC", 1);
    throw_required(machine, "INC", 0, OBJECT_INTEGER);
    int64_t n;
    machine.pop(n);
    ++n;
    machine.push(n);
}

// int => int
void DEC(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "DEC: Decrement" << std::endl;
        std::cout << "int DEC => int" << std::endl;
        return;
    }

    stack_required(machine, "DEC", 1);
    throw_required(machine, "DEC", 0, OBJECT_INTEGER);
    int64_t n;
    machine.pop(n);
    --n;
    machine.push(n);
}



