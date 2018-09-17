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

// obj =>
void DROP(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "DROP: Pop the item at L0";
        machine.helpstrm() << "obj DROP => ";
        return;
    }

    if (machine.stack_.empty())
        throw std::runtime_error("stack underflow");
    machine.stack_.pop_back();
}

// obj, obj... int =>
void DROPN(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "DROPN: Pop n items from the stack";
        machine.helpstrm() << "obj1 obj2 obj3... nitems DROPN =>";
        return;
    }

    int64_t n;
    machine.pop(n);
    if (machine.stack_.size() < n)
        throw std::runtime_error("stack underflow");
    while (n--)
        machine.stack_.pop_back();
}

// obj1 obj0 => obj0 obj1
void SWAP(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "SWAP: Swap the rop two objects on the stack";
        machine.helpstrm() << "obj1 obj2 SWAP => obj2 obj1";
        return;
    }

    if (machine.stack_.size() < 2)
        throw std::runtime_error("stack underflow");
    ObjectPtr o1, o2;
    machine.pop(o1);
    machine.pop(o2);
    machine.push(o1);
    machine.push(o2);
}

// obj => obj obj
void DUP(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "DUP: Duplicate the toptwo objects on the stack";
        machine.helpstrm() << "obj DUP => obj obj";
        return;
    }

    ObjectPtr optr = machine.peek(0);
    machine.push(optr);
}

void PICK(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "PICK: Copy the nth item from the stack and copy to L0";
        machine.helpstrm() << "obj1 obj2 level PICK => obj";
        return;
    }

    int64_t level;
    machine.pop(level);

    if (machine.stack_.size() <= level)
    {
        machine.push(level);
        throw std::runtime_error("PICK: stack underflow");
    }
    int64_t idx = machine.stack_.size() - level - 1;
    if (idx < 0 || idx >= machine.stack_.size())
    {
        machine.push(level);
        throw std::runtime_error("PICK: Out of range");
    }
    machine.push(machine.stack_[idx]);
}

void ROLL(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "ROLL: Move the nth item from the stack to L0";
        machine.helpstrm() << "obj1 obj2... nitem ROLL => obj";
        return;
    }

    int64_t level;
    machine.pop(level);

    if (machine.stack_.size() <= level)
    {
        machine.push(level);
        throw std::runtime_error("ROLL: stack underflow");
    }
    int64_t idx = machine.stack_.size() - level - 1;
    if (idx < 0 || idx >= machine.stack_.size())
    {
        machine.push(level);
        throw std::runtime_error("ROLL: Out of range");
    }
    ObjectPtr obj = machine.stack_[idx];
    machine.stack_.erase(machine.stack_.begin()+idx);
    machine.stack_.push_back(obj);
}

void VIEW(Machine& machine, size_t depth)
{
    if (machine.stack_.size() == 0)
        return;
    int n = std::min(depth, machine.stack_.size() - 1);
    while(n >= 0)
    {
        std::stringstream strm;
        ToStr(machine, machine.peek(n), strm, true);
        std::string s = strm.str();
        s = s.substr(0, machine.viewwidth);
        std::cout << n << ":" << s << std::endl;
        --n;
    }
}

void VIEW(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "VIEW: View the items in the stack";
        machine.helpstrm() << "obj1 obj2... VIEW => obj1 obj2...";
        return;
    }

    VIEW(machine, 20);
}

void CLRSTK(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "CLRSTK: Clear the stack";
        machine.helpstrm() << "obj1 obj2 obj3... CLRSTK =>";
        return;
    }

    machine.stack_.clear();
}

void DEPTH(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "DEPTH: Pushes the number of items on the stack";
        machine.helpstrm() << "obj1 obj2 ob3... DEPTH => obj1 obj2 obj3... int";
        return;
    }

    machine.push(machine.stack_.size());
}



