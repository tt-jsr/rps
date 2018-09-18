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


void NAMESPACES(Machine& machine)
{
    if (machine.help)
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
        sp->value = pr.first;
        lp->items.push_back(sp);
    }
    machine.push(lp);
}

void SETNS(Machine& machine)
{
    if (machine.help)
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
    if (machine.help)
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
    if (machine.help)
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

