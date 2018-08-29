#include <vector>
#include <unordered_map>
#include <exception>
#include <iostream>
#include <sstream>
#include "token.h"
#include "object.h"
#include "module.h"
#include "machine.h"

void ADD(Machine& machine)
{
    int64_t arg1, arg2;
    if (machine.stack_.size() < 2)
        throw std::runtime_error("ADD requires two arguments");
    if (machine.peek(0)->type != OBJECT_INTEGER)
        throw std::runtime_error("ADD requires INTEGER arguments");
    if (machine.peek(1)->type != OBJECT_INTEGER)
        throw std::runtime_error("ADD requires INTEGER arguments");
    machine.pop(arg1);
    machine.pop(arg2);
    machine.push(arg1 + arg2);
}



