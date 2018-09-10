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

void PRINT(Machine& machine)
{
    if (machine.stack_.size() < 1)
        throw std::runtime_error("PRINT: Requires object at L0");

    ObjectPtr optr;
    machine.pop(optr);

    std::cout << ToStr(machine, optr) << std::endl;
}

void PROMPT(Machine& machine)
{
    if (machine.stack_.size() < 1)
        throw std::runtime_error("INPUT: Requires string at L0");

    throw_required(machine, "PROMPT", 0, OBJECT_INTEGER);

    std::string prompt;
    machine.pop(prompt);

    std::cout << prompt << std::flush;

    std::string inbuf;
    std::getline(std::cin, inbuf);
    machine.push(inbuf);
}

