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

void DROP(Machine& machine)
{
    if (machine.stack_.empty())
        throw std::runtime_error("stack underflow");
    machine.stack_.pop_back();
}

void DROPN(Machine& machine)
{
    int64_t n;
    machine.pop(n);
    if (machine.stack_.size() < n)
        throw std::runtime_error("stack underflow");
    while (n--)
        machine.stack_.pop_back();
}

void SWAP(Machine& machine)
{
    if (machine.stack_.size() < 2)
        throw std::runtime_error("stack underflow");
    ObjectPtr o1, o2;
    machine.pop(o1);
    machine.pop(o2);
    machine.push(o1);
    machine.push(o2);
}

void DUP(Machine& machine)
{
    ObjectPtr optr = machine.peek(0);
    machine.push(optr);
}

void PICK(Machine& machine)
{
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

void VIEW(Machine& machine)
{
    size_t n = machine.stack_.size() - 1;;
    for (auto it = machine.stack_.begin(); it != machine.stack_.end(); ++it)
    {
        std::cout << n << ":" << ToStr(*it) << std::endl;
        --n;
    }
}




