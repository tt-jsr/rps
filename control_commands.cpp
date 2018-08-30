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

void IFT(Machine& machine)
{
    if (machine.stack_.size() < 2)
        throw std::runtime_error("IFT: stack underflow");
    ObjectPtr cond;
    ObjectPtr then;
    machine.pop(then);
    machine.pop(cond);
    EVAL(machine, cond);
    int64_t result;
    machine.pop(result);
    if (result)
        EVAL(machine, then);
}

void IFTE(Machine& machine)
{
    if (machine.stack_.size() < 2)
        throw std::runtime_error("IFTE: stack underflow");
    ObjectPtr cond;
    ObjectPtr then;
    ObjectPtr els;
    machine.pop(els);
    machine.pop(then);
    machine.pop(cond);
    EVAL(machine, cond);
    int64_t result;
    machine.pop(result);
    if (result)
        EVAL(machine, then);
    else
        EVAL(machine, els);
}


