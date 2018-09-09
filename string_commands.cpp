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

void FORMAT(Machine& machine)
{
    if (machine.stack_.size() < 1)
        throw std::runtime_error("FORMAT: Requires format at L0");
    if (machine.peek(0)->type != OBJECT_STRING)
        throw std::runtime_error("FORMAT: Requires format at L0");

    std::string fmt;
    machine.pop(fmt);

    char *p = &fmt[0];
    std::stringstream strm;

    int stackPopTo = 0;
    while (*p)
    {
        if (*p == '%')
        {
            ++p;
            if (isdigit(*p))
            {
                int n = strtol(p, &p, 10);
                if (n >= machine.stack_.size())
                    throw std::runtime_error("FORMAT: Stack out of bounds");
                stackPopTo = std::max(n, stackPopTo);
                strm << ToStr(machine, machine.peek(n));
            }
            else if (isalpha(*p))
            {
                std::string varname;
                while (*p && *p != ' ')
                {
                    varname.push_back(*p);
                    ++p;
                }
                ObjectPtr optr;
                try
                {
                    RCLL(machine, varname, optr);
                    strm << ToStr(machine, optr);
                }
                catch(std::exception&)
                {
                    try 
                    {
                        RCL(machine, varname, optr);
                        strm << ToStr(machine, optr);
                    }
                    catch(std::exception&)
                    {
                        strm << "nil";
                    }
                }
            }
        }
        else
        {
            strm << *p;
            ++p;
        }
    }
    machine.push(strm.str());
}

