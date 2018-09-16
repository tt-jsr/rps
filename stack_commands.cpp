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
        std::cout << "DROP: Pop the item at L0" << std::endl;
        std::cout << "obj DROP => " << std::endl;
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
        std::cout << "DROPN: Pop n items from the stack" << std::endl;
        std::cout << "obj1 obj2 obj3... nitems DROPN =>" << std::endl;
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
        std::cout << "SWAP: Swap the rop two objects on the stack" << std::endl;
        std::cout << "obj1 obj2 SWAP => obj2 obj1" << std::endl;
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
        std::cout << "DUP: Duplicate the toptwo objects on the stack" << std::endl;
        std::cout << "obj DUP => obj obj" << std::endl;
        return;
    }

    ObjectPtr optr = machine.peek(0);
    machine.push(optr);
}

void PICK(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "PICK: Copy the nth item from the stack and copy to L0" << std::endl;
        std::cout << "obj1 obj2 level PICK => obj" << std::endl;
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
        std::cout << "ROLL: Move the nth item from the stack to L0" << std::endl;
        std::cout << "obj1 obj2... nitem ROLL => obj" << std::endl;
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
        s = s.substr(0, machine.maxwidth);
        std::cout << n << ":" << s << std::endl;
        --n;
    }
}

void VIEW(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "VIEW: View the items in the stack" << std::endl;
        std::cout << "obj1 obj2... VIEW => obj1 obj2..." << std::endl;
        return;
    }

    VIEW(machine, 20);
}

void CLRSTK(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "CLRSTK: Clear the stack" << std::endl;
        std::cout << "obj1 obj2 obj3... CLRSTK =>" << std::endl;
        return;
    }

    machine.stack_.clear();
}

void DEPTH(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "DEPTH: Pushes the number of items on the stack" << std::endl;
        std::cout << "obj1 obj2 ob3... DEPTH => obj1 obj2 obj3... int" << std::endl;
        return;
    }

    machine.push(machine.stack_.size());
}



