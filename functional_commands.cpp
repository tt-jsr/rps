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

void APPLY(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "APPLY: Apply a program to each item in a list.";
        machine.helpstrm() << "       Returns a list the same size as the input.";
        machine.helpstrm() << "[srclist] <<prog>> APPLY => [dstlist]";
        machine.helpstrm() << "srclist: List of items";
        machine.helpstrm() << "prog: Program to execute. The program will have a list item at L0";
        machine.helpstrm() << "      the program will return an object to be placed on the dstlist";
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
        machine.helpstrm() << "SELECT Select items from a list";
        machine.helpstrm() << "       Returns a list of items selected from the input list.";
        machine.helpstrm() << "[srclist] <<prog>> SELECT => [dstlist]";
        machine.helpstrm() << "srclist: List of items";
        machine.helpstrm() << "prog: Program to execute. The program will have a list item at L0";
        machine.helpstrm() << "      the program will return true or false to have the item placed";
        machine.helpstrm() << "      on the dest list" << std::endl;
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

void MAP(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "MAP a function over a list";
        machine.helpstrm() << "[list] <<prog>> MAP => ";
        machine.helpstrm() << "srclist: List of items";
        machine.helpstrm() << "prog: Program to execute. The program will have a list item at L0";
        machine.helpstrm() << "MAP itself does not push anything on the stack, however the executed program may.";
        return;
    }

    stack_required(machine, "MAP", 2);
    throw_required(machine, "MAP", 0, OBJECT_PROGRAM);
    throw_required(machine, "MAP", 1, OBJECT_LIST);

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
    }
}

void REDUCE(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "REDUCE a function over a list";
        machine.helpstrm() << "[list] <<prog>> startobj REDUCE => obj ";
        machine.helpstrm() << "REDUCE calls prog with a list item and an object";
        machine.helpstrm() << "The program will then return an object to be provided";
        machine.helpstrm() << "to the next invokation of the program";
        machine.helpstrm() << "<<prog>> must have signiture:";
        machine.helpstrm() << "   [list] obj => obj";
        return;
    }

    stack_required(machine, "REDUCE", 3);
    throw_required(machine, "REDUCE", 1, OBJECT_PROGRAM);
    throw_required(machine, "REDUCE", 2, OBJECT_LIST);

    ProgramPtr prog;
    ListPtr list;
    ObjectPtr obj;
    machine.pop(obj);
    machine.pop(prog);
    machine.pop(list);
    for (ObjectPtr p : list->items)
    {
        machine.push(p);
        machine.push(obj);
        EVAL(machine, prog);
        machine.pop(obj);
    }
    machine.push(obj);
}


