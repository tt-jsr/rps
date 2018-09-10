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
    throw_required(machine, "FORMAT", 0, OBJECT_INTEGER);

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

void CAT(Machine& machine)
{
    stack_required(machine, "CAT", 2);
    throw_required(machine, "CAT", 0, OBJECT_STRING);
    throw_required(machine, "CAT", 1, OBJECT_STRING);

    std::string s1, s2;
    machine.pop(s1);
    machine.pop(s2);
    machine.push(s2+s1);
}

void JOIN(Machine& machine)
{
    stack_required(machine, "JOIN", 2);
    throw_required(machine, "JOIN", 1, OBJECT_LIST);
    throw_required(machine, "JOIN", 0, OBJECT_STRING);

    std::string delim;
    ListPtr lp;
    machine.pop(delim);
    machine.pop(lp);
    std::stringstream strm;
    void *last = lp->items.back().get();
    for (ObjectPtr op : lp->items)
    {
        std::string s = ToStr(machine, op);
        strm << s;
        if (op.get() != last)
            strm << delim;
    }
    machine.push(strm.str());
}

void SUBSTR(Machine& machine)
{
    stack_required(machine, "SUBSTR", 3);
    throw_required(machine, "SUBSTR", 0, OBJECT_INTEGER);
    throw_required(machine, "SUBSTR", 1, OBJECT_INTEGER);
    throw_required(machine, "SUBSTR", 2, OBJECT_STRING);

    int64_t startpos, length;
    std::string str;
    machine.pop(length);
    machine.pop(startpos);
    machine.pop(str);

    std::string s = str.substr(startpos, length);
    machine.push(s);
}

void STRFIND(Machine& machine)
{
    stack_required(machine, "FIND_FIRST", 3);
    throw_required(machine, "FIND_FIRST", 0, OBJECT_STRING);
    throw_required(machine, "FIND_FIRST", 1, OBJECT_INTEGER);
    throw_required(machine, "FIND_FIRST", 2, OBJECT_STRING);

    int64_t startpos;
    std::string find_str;
    std::string str;
    machine.pop(find_str);
    machine.pop(startpos);
    machine.pop(str);

    size_t pos = str.find(find_str, startpos);
    if (pos == std::string::npos)
        machine.push(-1);
    else
        machine.push(pos);
}

