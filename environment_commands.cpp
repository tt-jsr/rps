#include <vector>
#include <unordered_map>
#include <exception>
#include <iostream>
#include <sstream>
#include <cassert>
#include <unistd.h>
#include "token.h"
#include "object.h"
#include "module.h"
#include "machine.h"
#include "commands.h"
#include "utilities.h"
#include "parser.h"

namespace rps
{


void NAMESPACES(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "NAMESPACES: List namespaces";
        machine.helpstrm() << "NAMESPACES => [list]";
        machine.helpstrm() << "See also: SETNS GETNS =>";
        return;
    }
    ListPtr lp = MakeList();
    for (auto& pr : machine.modules_)
    {
        StringPtr sp = MakeString();
        sp->set(pr.first);
        lp->items.push_back(sp);
    }
    machine.push(lp);
}

void SETNS(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "SETNS: Create or change namespace";
        machine.helpstrm() << "\"namespace\" SETNS =>";
        machine.helpstrm() << "See also: GETNS NAMESPACES =>";
        return;
    }
    stack_required(machine, "SETNS", 1);
    throw_required(machine, "SETNS", 0, OBJECT_STRING);

    std::string s;
    machine.pop(s);
    machine.CreateModule(s);
    machine.current_module_ = s;
}

void GETNS(Machine& machine)
{ 
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "GETNS: Push current namespace";
        machine.helpstrm() << "GETNS => \"namespace\"";
        machine.helpstrm() << "See also: SETNS NAMESPACES =>";
        return;
    }
    machine.push(machine.current_module_);
}

void CD(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "CD: Change current dir";
        machine.helpstrm() << "\"dir\" CD =>";
        return;
    }
    stack_required(machine, "CD", 1);
    throw_required(machine, "CD", 0, OBJECT_STRING);

    std::string dir;
    machine.pop(dir);
    chdir(dir.c_str());
}

void PWD(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "PWD: Present wqorking directory";
        machine.helpstrm() << "PWD => \"dir\"";
        return;
    }

    char *p = getcwd(nullptr, 0);
    std::string s(p);
    free(p);
    machine.push(s);
}

void SETPROPERTY(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "SETPROPERTY: Set a property";
        machine.helpstrm() << "name value SETPROPERTY => ";
        return;
    }

    stack_required(machine, "SETPROPERTY", 2);
    throw_required(machine, "SETPROPERTY", 1, OBJECT_STRING);

    ObjectPtr value;
    std::string name;

    machine.pop(value);
    machine.pop(name);
    machine.SetProperty(name, value);
}

void GETPROPERTY(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "GETPROPERTY: Get a property";
        machine.helpstrm() << "name GETPROPERTY => value";
        return;
    }

    stack_required(machine, "GETPROPERTY", 1);
    throw_required(machine, "GETPROPERTY", 0, OBJECT_STRING);

    std::string name;

    machine.pop(name);
    ObjectPtr optr = machine.GetProperty(name);
    if (optr)
        machine.push(optr);
}

void IMPORT(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "IMPORT: Import and execute a file";
        machine.helpstrm() << "name IMPORT => ";
        return;
    }

    stack_required(machine, "IMPORT", 1);
    throw_required(machine, "IMPORT", 0, OBJECT_STRING);

    std::string name;

    machine.pop(name);

    Import(machine, name);
}

} // namespace rps

