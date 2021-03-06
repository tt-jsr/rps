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

namespace rps
{

void TOINT(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "TOINT: Convert to integer";
        machine.helpstrm() << "obj TOINT => int";
        return;
    }

    ObjectPtr optr;
    stack_required(machine, "TOINT", 1);

    machine.pop(optr);
    IntegerPtr ip = MakeInteger();
    ip->value = ToInt(machine, optr);
    optr = ip;
    machine.push(optr);
}

void TOSTR(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "TOSTR: Convert to string";
        machine.helpstrm() << "obj TOSTR => \"str\"";
        return;
    }

    ObjectPtr optr;
    stack_required(machine, "TOSTR", 1);

    machine.pop(optr);
    StringPtr sp = MakeString();
    sp->set(ToStr(machine, optr));
    optr = sp;
    machine.push(optr);
}

void TYPE(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "TYPE: Pushes the type of object at L0";
        machine.helpstrm() << "obj TYPE => \"str\"";
        return;
    }

    ObjectPtr optr;
    stack_required(machine, "TYPE", 1);

    machine.pop(optr);
    StringPtr sp = MakeString();
    sp->set(ToType(machine, optr));
    optr = sp;
    machine.push(optr);
}

void CLONE(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "CLONE: Clone the object at L0";
        machine.helpstrm() << "obj CLONE => obj obj";
        return;
    }
    stack_required(machine, "CLONE", 1);

    ObjectPtr optr = Clone(machine.stack_[machine.stack_.size()-1]);
    machine.push(optr);
}


} // namespace rps

