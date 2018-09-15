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

// "str" => int
// int  => int
void TOINT(Machine& machine)
{
    ObjectPtr optr;
    stack_required(machine, "TOINT", 1);

    machine.pop(optr);
    IntegerPtr ip = MakeInteger();
    ip->value = ToInt(machine, optr);
    optr = ip;
    machine.push(optr);
}

// "str" => "str"
// int  => "str"
void TOSTR(Machine& machine)
{
    ObjectPtr optr;
    stack_required(machine, "TOSTR", 1);

    machine.pop(optr);
    StringPtr sp = MakeString();
    sp->value = ToStr(machine, optr);
    optr = sp;
    machine.push(optr);
}

// obj => "str"
void TYPE(Machine& machine)
{
    ObjectPtr optr;
    stack_required(machine, "TYPE", 1);

    machine.pop(optr);
    StringPtr sp = MakeString();
    sp->value = ToType(machine, optr);
    optr = sp;
}

