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

namespace rps
{

void APPLY(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "APPLY: Apply a program to each item in a list.";
        machine.helpstrm() << "       Returns a list the same size as the input.";
        machine.helpstrm() << "[srclist] <<prog>> APPLY => [dstlist]";
        machine.helpstrm() << "[srclist] \"progname\" APPLY => [dstlist]";
        machine.helpstrm() << "srclist: List of items";
        machine.helpstrm() << "prog: Program to execute. The program will have a list item at L0";
        machine.helpstrm() << "      the program will return an object to be placed on the dstlist";
        return;
    }
    stack_required(machine, "APPLY", 2);
    throw_required(machine, "APPLY", 1, OBJECT_LIST);

    ListPtr result = MakeList();
    ListPtr lp;
    ObjectPtr optr;
    ProgramPtr pptr;

    machine.pop(optr);
    if (optr->type == OBJECT_STRING)
    {
        std::string name = ((String *)optr.get())->get();
        RCL(machine, name, optr);
    }
    if (optr->type != OBJECT_PROGRAM)
        throw std::runtime_error("APPLY: Program or program name must be at level 0");

    pptr = std::static_pointer_cast<Program>(optr);

    machine.pop(lp);

    for (ObjectPtr p : lp->items)
    {
        if (bInterrupt)
            return;
        machine.push(p);
        EVAL(machine, pptr);
        machine.pop(optr);
        result->items.push_back(optr);
    }
    machine.push(result);
}

void APPLY1(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "APPLY1: Apply a program to each item in a list with an argument.";
        machine.helpstrm() << "       Returns a list the same size as the input.";
        machine.helpstrm() << "[srclist] <<prog>> argObj APPLY1 => [dstlist]";
        machine.helpstrm() << "[srclist] \"progname\" argObj APPLY1 => [dstlist]";
        machine.helpstrm() << "srclist: List of items";
        machine.helpstrm() << "prog: Program to execute. The program will have a list item at L1 and argObj at L0";
        machine.helpstrm() << "      the program will return an object to be placed on the dstlist";
        return;
    }
    stack_required(machine, "APPLY", 3);
    throw_required(machine, "APPLY", 2, OBJECT_LIST);

    ListPtr result = MakeList();
    ListPtr lp;
    ObjectPtr optr;
    ObjectPtr arg;
    ProgramPtr pptr;

    machine.pop(arg);
    machine.pop(optr);
    if (optr->type == OBJECT_STRING)
    {
        std::string name = ((String *)optr.get())->get();
        RCL(machine, name, optr);
    }
    if (optr->type != OBJECT_PROGRAM)
        throw std::runtime_error("APPLY: Program or program name must be at level 1");

    pptr = std::static_pointer_cast<Program>(optr);

    machine.pop(lp);

    for (ObjectPtr p : lp->items)
    {
        if (bInterrupt)
            return;
        machine.push(p);
        machine.push(arg);
        EVAL(machine, pptr);
        machine.pop(optr);
        result->items.push_back(optr);
    }
    machine.push(result);
}

void FILTER(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "FILTER Filter items from a list";
        machine.helpstrm() << "       Returns a list of items removed from the input list.";
        machine.helpstrm() << "[srclist] <<prog>> FILTER => [dstlist]";
        machine.helpstrm() << "srclist: List of items";
        machine.helpstrm() << "prog: Program to execute. The program will have a list item at L0";
        machine.helpstrm() << "      the program will return true or false to have the item removed";
        machine.helpstrm() << "      on the dest list" << std::endl;
        return;
    }

    stack_required(machine, "FILTER", 2);
    throw_required(machine, "FILTER", 1, OBJECT_LIST);

    ListPtr result = MakeList();
    ListPtr lp;
    ObjectPtr optr;
    ProgramPtr pptr;

    machine.pop(optr);
    if (optr->type == OBJECT_STRING)
    {
        std::string name = ((String *)optr.get())->get();
        RCL(machine, name, optr);
    }
    if (optr->type != OBJECT_PROGRAM)
        throw std::runtime_error("FILTER: Program or program name must be at level 0");
    pptr = std::static_pointer_cast<Program>(optr);

    machine.pop(lp);

    for (ObjectPtr p : lp->items)
    {
        if (bInterrupt)
            return;
        machine.push(p);
        EVAL(machine, pptr);
        machine.pop(optr);
        if (ToBool(machine, optr) == false)
            result->items.push_back(p);
    }
    machine.push(result);
}

void FILTER1(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "FILTER1 Remove items from a list with an argument";
        machine.helpstrm() << "       Returns a list of items removed from the input list.";
        machine.helpstrm() << "[srclist] <<prog>> argObj FILTER1 => [dstlist]";
        machine.helpstrm() << "srclist: List of items";
        machine.helpstrm() << "prog: Program to execute. The program will have a list item at L1 and argObj at L0";
        machine.helpstrm() << "      the program will return true or false to have the item removed";
        machine.helpstrm() << "      on the dest list" << std::endl;
        return;
    }

    stack_required(machine, "SELECT", 3);
    throw_required(machine, "SELECT", 2, OBJECT_LIST);

    ListPtr result = MakeList();
    ListPtr lp;
    ObjectPtr optr;
    ObjectPtr arg;
    ProgramPtr pptr;

    machine.pop(arg);
    machine.pop(optr);
    if (optr->type == OBJECT_STRING)
    {
        std::string name = ((String *)optr.get())->get();
        RCL(machine, name, optr);
    }
    if (optr->type != OBJECT_PROGRAM)
        throw std::runtime_error("SELECT: Program or program name must be at level 1");
    pptr = std::static_pointer_cast<Program>(optr);

    machine.pop(lp);

    for (ObjectPtr p : lp->items)
    {
        if (bInterrupt)
            return;
        machine.push(p);
        machine.push(arg);
        EVAL(machine, pptr);
        machine.pop(optr);
        if (ToBool(machine, optr) == false)
            result->items.push_back(p);
    }
    machine.push(result);
}

void MAP(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "MAP a function over a list";
        machine.helpstrm() << "[list] <<prog>> opt MAP => ";
        machine.helpstrm() << "[list] \"progname\" opt MAP => ";
        machine.helpstrm() << "srclist: List of items";
        machine.helpstrm() << "prog: Program to execute. The program will have a list item at L0";
        machine.helpstrm() << "opt: Optional options.";
        machine.helpstrm() << "     --query: Prompt the user to continue";
        machine.helpstrm() << "MAP itself does not push anything on the stack, however the executed program may.";
        return;
    }

    stack_required(machine, "MAP", 2);

    ListPtr result = MakeList();
    ListPtr lp;
    ObjectPtr optr;
    ProgramPtr pptr;
    std::vector<std::string> args;
    GetArgs(machine, args);
    bool query(false);
    for (auto& arg : args)
    {
        if (arg == "--query")
            query = true;
    }

    throw_required(machine, "MAP", 1, OBJECT_LIST);
    machine.pop(optr);
    if (optr->type == OBJECT_STRING)
    {
        std::string name = ((String *)optr.get())->get();
        RCL(machine, name, optr);
    }
    if (optr->type != OBJECT_PROGRAM)
        throw std::runtime_error("MAP: Program or program name must be at level 0");
    pptr = std::static_pointer_cast<Program>(optr);

    machine.pop(lp);

    std::string input;
    for (ObjectPtr p : lp->items)
    {
        if (bInterrupt)
            return;
        machine.push(p);
        EVAL(machine, pptr);
        if (query)
        {
            std::cout << "Continue (y/n)" << std::flush;
            std::getline(std::cin, input);
            if (input[0] == 'n')
                return;
        }
    }
}

void MAP1(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "MAP1 a function over a list with an argument";
        machine.helpstrm() << "[list] <<prog>> argObj opt MAP => ";
        machine.helpstrm() << "[list] \"progname\" argObj opt MAP => ";
        machine.helpstrm() << "srclist: List of items";
        machine.helpstrm() << "prog: Program to execute. The program will have a list item at L1 and argObj at L0";
        machine.helpstrm() << "opt: Optional options.";
        machine.helpstrm() << "     --query: Prompt the user to continue";
        machine.helpstrm() << "MAP1 itself does not push anything on the stack, however the executed program may.";
        return;
    }

    stack_required(machine, "MAP1", 3);

    ListPtr result = MakeList();
    ListPtr lp;
    ObjectPtr optr;
    ProgramPtr pptr;
    ObjectPtr arg;
    std::vector<std::string> args;
    GetArgs(machine, args);
    bool query(false);
    for (auto& arg : args)
    {
        if (arg == "--query")
            query = true;
    }

    throw_required(machine, "MAP1", 2, OBJECT_LIST);
    machine.pop(arg);

    machine.pop(optr);
    if (optr->type == OBJECT_STRING)
    {
        std::string name = ((String *)optr.get())->get();
        RCL(machine, name, optr);
    }
    if (optr->type != OBJECT_PROGRAM)
        throw std::runtime_error("MAP: Program or program name must be at level 1");
    pptr = std::static_pointer_cast<Program>(optr);

    machine.pop(lp);

    std::string input;
    for (ObjectPtr p : lp->items)
    {
        if (bInterrupt)
            return;
        machine.push(p);
        machine.push(arg);
        EVAL(machine, pptr);
        if (query)
        {
            std::cout << "Continue (y/n)" << std::flush;
            std::getline(std::cin, input);
            if (input[0] == 'n')
                return;
        }
    }
}

void REDUCE(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "REDUCE a list to an object";
        machine.helpstrm() << "[list] <<prog>> startobj REDUCE => obj ";
        machine.helpstrm() << "[list] \"progname\" startobj REDUCE => obj ";
        machine.helpstrm() << "REDUCE calls prog with a list item and the startobj.";
        machine.helpstrm() << "The program will then return an object to be provided";
        machine.helpstrm() << "to the next invocation of the program";
        machine.helpstrm() << "<<prog>> must have signiture:";
        machine.helpstrm() << "   listitem obj => obj";
        return;
    }

    stack_required(machine, "REDUCE", 3);
    throw_required(machine, "REDUCE", 2, OBJECT_LIST);

    ObjectPtr prog;
    ListPtr list;
    ObjectPtr obj;
    machine.pop(obj);
    machine.pop(prog);
    if (prog->type == OBJECT_STRING)
    {
        std::string name = ((String *)prog.get())->get();
        RCL(machine, name, prog);
    }
    if (prog->type != OBJECT_PROGRAM)
        throw std::runtime_error("REDUCE: Program or program name must be at level 0");

    ProgramPtr pptr;
    pptr = std::static_pointer_cast<Program>(prog);
    machine.pop(list);
    for (ObjectPtr p : list->items)
    {
        if (bInterrupt)
            return;
        machine.push(p);
        machine.push(obj);
        EVAL(machine, pptr);
        machine.pop(obj);
    }
    machine.push(obj);
}


} // namespace rps

