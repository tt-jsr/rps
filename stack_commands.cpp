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

void DROP(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "DROP: Pop the item at L0";
        machine.helpstrm() << "obj DROP => ";
        return;
    }

    stack_required(machine, "DROP", 1);
    machine.pop();
}

void DROPN(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "DROPN: Pop n items from the stack";
        machine.helpstrm() << "obj1 obj2 obj3... nitems DROPN =>";
        return;
    }

    stack_required(machine, "DROPN", 1);

    int64_t n;
    machine.pop(n);
    stack_required(machine, "DROPN", n);
    while (n--)
        machine.pop();
}

void SWAP(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "SWAP: Swap the top two objects on the stack";
        machine.helpstrm() << "obj1 obj2 SWAP => obj2 obj1";
        return;
    }

    stack_required(machine, "SWAP", 2);

    ObjectPtr o1, o2;
    machine.pop(o1);
    machine.pop(o2);
    machine.push(o1);
    machine.push(o2);
}

void DUP(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "DUP: Duplicate the toptwo objects on the stack";
        machine.helpstrm() << "obj DUP => obj obj";
        return;
    }

    stack_required(machine, "DUP", 1);

    ObjectPtr optr = machine.peek(0);
    machine.push(optr);
}

void PICK(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "PICK: Copy the nth item from the stack and copy to L0";
        machine.helpstrm() << "obj1 obj2 level PICK => obj";
        return;
    }

    int64_t level;
    machine.pop(level);

    stack_required(machine, "PICK", level);
    int64_t idx = machine.stack_.size() - level - 1;

    machine.push(machine.stack_[idx]);
}

void ROLL(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "ROLL: Move the nth item from the stack to L0";
        machine.helpstrm() << "obj1 obj2 obj3... nitem ROLL => obj2 obj3 obj1";
        return;
    }

    int64_t level;
    machine.pop(level);

    stack_required(machine, "ROLL", level);
    int64_t idx = machine.stack_.size() - level - 1;

    ObjectPtr obj = machine.stack_[idx];
    machine.stack_.erase(machine.stack_.begin()+idx);
    machine.stack_.push_back(obj);
}

void ROLLD(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "ROLLD: Move L0 item from the stack to nth level";
        machine.helpstrm() << "obj1 obj2 obj3... nitem ROLLD => obj3 obj2 obj1";
        return;
    }

    int64_t level;
    machine.pop(level);
    ObjectPtr optr;

    stack_required(machine, "ROLLD", level);

    int64_t idx = machine.stack_.size() - level - 1;

    machine.pop(optr);
    machine.stack_.insert(machine.stack_.begin()+idx, optr);
}

void VIEW(Machine& machine, size_t depth)
{
    int64_t viewwidth = machine.GetProperty("viewwidth", 120);
    if (machine.stack_.size() == 0)
        return;
    int n = std::min(depth, machine.stack_.size() - 1);
    while(n >= 0)
    {
        std::string s = ToStr(machine, machine.peek(n));
        s = s.substr(0, viewwidth);
        std::cout << n << ":" << s << std::endl;
        --n;
    }
}

void VIEW(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "VIEW: View the items in the stack";
        machine.helpstrm() << "obj1 obj2... VIEW => obj1 obj2...";
        return;
    }

    VIEW(machine, 20);
}

void CLRSTK(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "CLRSTK: Clear the stack";
        machine.helpstrm() << "obj1 obj2 obj3... CLRSTK =>";
        return;
    }

    machine.stack_.clear();
}

void DEPTH(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "DEPTH: Pushes the number of items on the stack";
        machine.helpstrm() << "obj1 obj2 ob3... DEPTH => obj1 obj2 obj3... int";
        return;
    }

    machine.push(machine.stack_.size());
}


} // namespace rps

