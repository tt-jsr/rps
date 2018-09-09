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
    if (machine.stack_.size() < 2)
        throw std::runtime_error("APPLY: Requires two arguments");
    throw_required(machine, "APPLY", 0, OBJECT_PROGRAM);
    throw_required(machine, "APPLY", 1, OBJECT_LIST);

    ListPtr result = MakeList();
    ListPtr lp;
    ObjectPtr optr;
    ProgramPtr pptr;

    machine.pop(optr);
    pptr = std::static_pointer_cast<Program>(optr);

    machine.pop(lp);

    size_t sz = machine.stack_.size();

    for (ObjectPtr p : lp->items)
    {
        machine.push(p);
        EVAL(machine, pptr);
        if (machine.stack_.size() > sz)
        {
            size_t n = machine.stack_.size() - sz;
            if (n == 1)
            {
                machine.pop(optr);
                result->items.push_back(optr);
            }
            if (n > 1)
            {
                ListPtr r = MakeList();
                while (n--)
                {
                    machine.pop(optr);
                    r->items.push_back(optr);
                }
                result->items.push_back(r);
            }
        }
    }
    if (result->items.size())
        machine.push(result);
}

