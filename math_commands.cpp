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
        machine.helpstrm() << "ADD: Addition";
        machine.helpstrm() << "int1 int2 ADD => int";
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
        machine.helpstrm() << "SUB: Subraction";
        machine.helpstrm() << "int1 int2 SUB => int";
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
        machine.helpstrm() << "MUL: Multilication";
        machine.helpstrm() << "int1 int2 MUL => int";
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
        machine.helpstrm() << "DIV: Division";
        machine.helpstrm() << "int1 int2 DIV => int";
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
        machine.helpstrm() << "INC: Increment";
        machine.helpstrm() << "int INC => int";
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
        machine.helpstrm() << "DEC: Decrement";
        machine.helpstrm() << "int DEC => int";
        return;
    }

    stack_required(machine, "DEC", 1);
    throw_required(machine, "DEC", 0, OBJECT_INTEGER);
    int64_t n;
    machine.pop(n);
    --n;
    machine.push(n);
}



