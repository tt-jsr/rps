#include <vector>
#include <unordered_map>
#include <exception>
#include <iostream>
#include <sstream>
#include <cassert>
#include <cstring>
#include "token.h"
#include "object.h"
#include "module.h"
#include "machine.h"
#include "commands.h"
#include "utilities.h"

// obj obj... "str" => "str"
void FORMAT(Machine& machine)
{
    stack_required(machine, "FORMAT", 1);
    throw_required(machine, "FORMAT", 0, OBJECT_STRING);

    std::string fmt;
    machine.pop(fmt);

    char *p = &fmt[0];
    std::stringstream strm;

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

// "str" "str" => "str"
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

// [list]  "str" => "str"
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

// "str" startpos length => "str"
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

// "str" startpos "str to find"  => int
void STRFIND(Machine& machine)
{
    stack_required(machine, "FIND", 3);
    throw_required(machine, "FIND", 0, OBJECT_STRING);
    throw_required(machine, "FIND", 1, OBJECT_INTEGER);
    throw_required(machine, "FIND", 2, OBJECT_STRING);

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

// "str" "str" => int
void STRCMP(Machine& machine)
{
    stack_required(machine, "STRCMP", 2);
    throw_required(machine, "STRCMP", 0, OBJECT_STRING);
    throw_required(machine, "STRCMP", 1, OBJECT_STRING);

    std::string s1, s2;
    machine.pop(s2);
    machine.pop(s1);
    
    int64_t n = std::strcmp(s1.c_str(), s2.c_str());
    machine.push(n);
}

// "str" "str" length => int
void STRNCMP(Machine& machine)
{
    stack_required(machine, "STRNCMP", 3);
    throw_required(machine, "STRNCMP", 0, OBJECT_INTEGER);
    throw_required(machine, "STRNCMP", 1, OBJECT_STRING);
    throw_required(machine, "STRNCMP", 2, OBJECT_STRING);

    std::string s1, s2;
    int64_t length;
    machine.pop(length);
    machine.pop(s2);
    machine.pop(s1);
    
    int64_t n = std::strncmp(s1.c_str(), s2.c_str(), length);
    machine.push(n);
}

void SPLIT(Machine& machine)
{
    stack_required(machine, "SPLIT", 2);
    throw_required(machine, "SPLIT", 0, OBJECT_STRING); // delim
    throw_required(machine, "SPLIT", 1, OBJECT_STRING); // string to split

    std::string str,  delims;
    ListPtr result = MakeList();
    bool bCollapse(false);
    int max = 10000;

    machine.pop(delims);
    while (strncmp(delims.c_str(), "--", 2) == 0)
    {
        if (isdigit(*(delims.c_str()+2)))
        {
            max = std::stol(delims.c_str()+2);
            machine.pop(delims);
        }
        if (delims == "--collapse" )
        {
            bCollapse = true;
            machine.pop(delims);
        }
    }
    machine.pop(str);
    std::string s;
    int nmatch = 0;
    for (auto it = str.begin(); it != str.end(); ++it)
    {
        if (delims.find_first_of(*it) != std::string::npos)
        {
            StringPtr sp = MakeString();
            sp->value = s;
            result->items.push_back(sp);
            s = "";
            ++nmatch;
            if (nmatch == max)
            {
                StringPtr sp = MakeString();
                ++it;
                sp->value = &*it;
                result->items.push_back(sp);
                break;
            }
            if (bCollapse)
            {
                while (delims.find_first_of(*it) != std::string::npos)
                    ++it;
                --it;
            }
        }
        else
        {
            s.push_back(*it);
        }
    }
    if (s.size())
    {
        StringPtr sp = MakeString();
        sp->value = s;
        result->items.push_back(sp);
    }
    ObjectPtr optr = result;
    machine.push(optr);
}


