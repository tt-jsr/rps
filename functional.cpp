#include <vector>
#include <unordered_map>
#include <exception>
#include <iostream>
#include <sstream>
#include <cassert>
#include "token.h"
#include "object.h"
#include "module.h"
#include "machine.h"
#include "commands.h"
#include "utilities.h"

// [list]   <<prog>>  =>  [list]
void APPLY(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "APPLY: Apply a program to each item in a list" << std::endl;
        std::cout << "[srclist] <<prog>> APPLY => [dstlist]" << std::endl;
        std::cout << "srclist: List of items" << std::endl;
        std::cout << "prog: Program to execute. The program will have a list item at L0" << std::endl;
        std::cout << "      the program will return an object to be placedon the dstlist" << std::endl;
        return;
    }
    stack_required(machine, "APPLY", 2);
    throw_required(machine, "APPLY", 0, OBJECT_PROGRAM);
    throw_required(machine, "APPLY", 1, OBJECT_LIST);

    ListPtr result = MakeList();
    ListPtr lp;
    ObjectPtr optr;
    ProgramPtr pptr;

    machine.pop(optr);
    pptr = std::static_pointer_cast<Program>(optr);

    machine.pop(lp);

    for (ObjectPtr p : lp->items)
    {
        machine.push(p);
        EVAL(machine, pptr);
        machine.pop(optr);
        result->items.push_back(optr);
    }
    machine.push(result);
}

void SELECT(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "SELECT Select items from a list" << std::endl;
        std::cout << "[srclist] <<prog>> SELECT => [dstlist]" << std::endl;
        std::cout << "srclist: List of items" << std::endl;
        std::cout << "prog: Program to execute. The program will have a list item at L0" << std::endl;
        std::cout << "      the program will return true or false to have the item placed" << std::endl;
        std::cout << "      on the dest list" << std::endl;
        return;
    }

    stack_required(machine, "SELECT", 2);
    throw_required(machine, "SELECT", 0, OBJECT_PROGRAM);
    throw_required(machine, "SELECT", 1, OBJECT_LIST);

    ListPtr result = MakeList();
    ListPtr lp;
    ObjectPtr optr;
    ProgramPtr pptr;

    machine.pop(optr);
    pptr = std::static_pointer_cast<Program>(optr);

    machine.pop(lp);

    for (ObjectPtr p : lp->items)
    {
        machine.push(p);
        EVAL(machine, pptr);
        machine.pop(optr);
        if (ToBool(machine, optr))
            result->items.push_back(p);
    }
    machine.push(result);
}
