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

namespace rps
{

void FORMAT(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "FORMAT: Format a string";
        machine.helpstrm() << "\"str\" FORMAT => \"str\"";
        machine.helpstrm() << "todo: Format description...";
        return;
    }

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
            std::string spec;
            ++p;
            if (*p == '{')
            {
                ++p;
                while(*p && *p != '}')
                {
                    spec.push_back(*p);
                    ++p;
                }
                ++p;
            }
            else
            {
                while(*p && *p != ' ')
                {
                    spec.push_back(*p);
                    ++p;
                }
            }
            if (isdigit(spec[0]))
            {
                int n = strtol(spec.c_str(), nullptr, 10);
                if (n >= machine.stack_.size())
                    throw std::runtime_error("FORMAT: Stack out of bounds");
                strm << ToStr(machine, machine.peek(n));
            }
            else 
            {
                ObjectPtr optr;
                try
                {
                    RCLL(machine, spec, optr);
                    strm << ToStr(machine, optr);
                }
                catch(std::exception&)
                {
                    try 
                    {
                        RCL(machine, spec, optr);
                        strm << ToStr(machine, optr);
                    }
                    catch(std::exception&)
                    {
                        strm << spec;
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
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "CAT: Concatenate two strings";
        machine.helpstrm() << "\"str1\" \"str2\" CAT => \"str1str2\"";
        return;
    }

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
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "JOIN: Joins a list into a string";
        machine.helpstrm() << "[list] \"delim\" JOIN =>\"str\"";
        machine.helpstrm() << "delim: The string to join with";
        return;
    }

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
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "SUBSTR: Return substring given startpos and length";
        machine.helpstrm() << "\"str\" startpos length SUBSTR => \"str\"";
        machine.helpstrm() << "If length is <= 0, return substr to end of string";
        return;
    }

    stack_required(machine, "SUBSTR", 3);
    throw_required(machine, "SUBSTR", 0, OBJECT_INTEGER);
    throw_required(machine, "SUBSTR", 1, OBJECT_INTEGER);
    throw_required(machine, "SUBSTR", 2, OBJECT_STRING);

    int64_t startpos, length;
    std::string str;
    machine.pop(length);
    machine.pop(startpos);
    machine.pop(str);
    if (length <= 0)
        str = str.substr(startpos);
    else
        str = str.substr(startpos, length);
    machine.push(str);
}

void SUBSTRPOS(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "SUBSTRPOS: Return substr given startpos and endpos";
        machine.helpstrm() << "\"str\" startpos endpos SUBSTRPOS => \"str\"";
        machine.helpstrm() << "If endpos is <= startpos, return substr to end of string";
        return;
    }

    stack_required(machine, "SUBSTRPOS", 3);
    throw_required(machine, "SUBSTRPOS", 0, OBJECT_INTEGER);
    throw_required(machine, "SUBSTRPOS", 1, OBJECT_INTEGER);
    throw_required(machine, "SUBSTRPOS", 2, OBJECT_STRING);

    int64_t startpos, endpos;
    std::string str;
    machine.pop(endpos);
    machine.pop(startpos);
    machine.pop(str);
    int64_t length = endpos - startpos;
    if (length < 0)
        str = str.substr(startpos);
    else
        str = str.substr(startpos, length);
    machine.push(str);
}

void STRBEGIN(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "STRBEGIN: Does string begin with the given str";
        machine.helpstrm() << "\"str\" \"str\" STRBEGIN => int";
        return;
    }

    stack_required(machine, "STRBEGIN", 2);
    throw_required(machine, "STRBEGIN", 0, OBJECT_STRING);
    throw_required(machine, "STRBEGIN", 1, OBJECT_STRING);

    std::string str;
    std::string cmp;
    machine.pop(cmp);
    machine.pop(str);
    if (cmp.size() > str.size())
    {
        machine.push(0);
        return;
    }
    if (strncmp(str.c_str(), cmp.c_str(), cmp.size()) == 0)
        machine.push(1);
    else
        machine.push(0);
}

void STREND(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "STREND: Does string end with the given str";
        machine.helpstrm() << "\"str\" \"str\" STREND => int";
        return;
    }

    stack_required(machine, "STREND", 2);
    throw_required(machine, "STREND", 0, OBJECT_STRING);
    throw_required(machine, "STREND", 1, OBJECT_STRING);

    std::string str;
    std::string cmp;
    machine.pop(cmp);
    machine.pop(str);
    int startpos = str.size() - cmp.size();
    if (startpos < 0)
    {
        machine.push(0);
        return;
    }
    if (strncmp(str.c_str()+startpos, cmp.c_str(), cmp.size()) == 0)
        machine.push(1);
    else
        machine.push(0);
}

void STRHAS(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "STRHAS: Does string contain the given str";
        machine.helpstrm() << "\"str\" \"str\" STRHAS => int";
        return;
    }

    stack_required(machine, "STRHAS", 2);
    throw_required(machine, "STRHAS", 0, OBJECT_STRING);
    throw_required(machine, "STRHAS", 1, OBJECT_STRING);

    std::string str;
    std::string find_str;
    machine.pop(find_str);
    machine.pop(str);

    size_t pos = str.find(find_str, 0);
    if (pos == std::string::npos)
        machine.push(0);
    else
        machine.push(1);

}

void STRFIND(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "STRFIND: Find a string";
        machine.helpstrm() << "\"str\" startpos \"str to find\" STRFIND => pos";
        machine.helpstrm() << "Pushes -1 if the string is not found";
        return;
    }

    stack_required(machine, "STRFIND", 3);
    throw_required(machine, "STRFIND", 0, OBJECT_STRING);
    throw_required(machine, "STRFIND", 1, OBJECT_INTEGER);
    throw_required(machine, "STRFIND", 2, OBJECT_STRING);

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

void STRFINDEND(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "STRFINDEND: Find the end of a string";
        machine.helpstrm() << "\"str\" startpos \"str to find\" STRFINDEND => pos";
        machine.helpstrm() << "Pushes -1 if the string is not found";
        machine.helpstrm() << "This function returns the position of the end of the found string";
        return;
    }

    stack_required(machine, "STRFINDEND", 3);
    throw_required(machine, "STRFINDEND", 0, OBJECT_STRING);
    throw_required(machine, "STRFINDEND", 1, OBJECT_INTEGER);
    throw_required(machine, "STRFINDEND", 2, OBJECT_STRING);

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
        machine.push(pos+find_str.size());
}

void STRCMP(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "STRCMP: Compare two strings";
        machine.helpstrm() << "\"str1\" \"str2\" STRCMP => int";
        machine.helpstrm() << "Pushes <0, 0 or >0";
        return;
    }

    stack_required(machine, "STRCMP", 2);
    throw_required(machine, "STRCMP", 0, OBJECT_STRING);
    throw_required(machine, "STRCMP", 1, OBJECT_STRING);

    std::string s1, s2;
    machine.pop(s2);
    machine.pop(s1);
    
    int64_t n = std::strcmp(s1.c_str(), s2.c_str());
    machine.push(n);
}

void STRNCMP(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "STRNCMP: Compare two strings for the given length";
        machine.helpstrm() << "\"str1\" \"str2\" length STRNCMP => int";
        machine.helpstrm() << "Pushes <0, 0 or >0";
        return;
    }

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
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "SPLIT: Split a string";
        machine.helpstrm() << "\"str\" \"delim\" opts SPLIT => [list]";
        machine.helpstrm() << "delims: Set of delimiter chars to split with";
        machine.helpstrm() << "opts: Optional arguments";
        machine.helpstrm() << "     --collapse: Merge empty strings";
        machine.helpstrm() << "     --n: Max number of splits";
        return;
    }

    stack_required(machine, "SPLIT", 2);

    std::string str,  delims;
    ListPtr result = MakeList();
    bool bCollapse(false);
    int max = 10000;

    std::vector<std::string> args;
    GetArgs(machine, args);
    for (auto& arg: args)
    {
        if (isdigit(*(arg.c_str()+2)))
        {
            max = std::stol(arg.c_str()+2);
        }
        if (arg == "--collapse" )
        {
            bCollapse = true;
        }
    }
    throw_required(machine, "SPLIT", 0, OBJECT_STRING); // delim
    throw_required(machine, "SPLIT", 1, OBJECT_STRING); // string to split
    machine.pop(delims);
    machine.pop(str);
    std::string s;
    int nmatch = 0;
    for (auto it = str.begin(); it != str.end(); ++it)
    {
        if (delims.find_first_of(*it) != std::string::npos)
        {
            StringPtr sp = MakeString();
            sp->set(s);
            result->items.push_back(sp);
            s = "";
            ++nmatch;
            if (nmatch == max)
            {
                StringPtr sp = MakeString();
                ++it;
                sp->set(&*it);
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
        sp->set(s);
        result->items.push_back(sp);
    }
    ObjectPtr optr = result;
    machine.push(optr);
}

void STRCSPN(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "STRCSPN: Returns pos of delimiter";
        machine.helpstrm() << "\"str\" startpos \"delims\" STRCSPN => int";
        return;
    }
    stack_required(machine, "STRCSPN", 3);
    throw_required(machine, "STRCSPN", 0, OBJECT_STRING);
    throw_required(machine, "STRCSPN", 1, OBJECT_INTEGER);
    throw_required(machine, "STRCSPN", 2, OBJECT_STRING);

    std::string delims;
    std::string str;
    int64_t startpos;
    machine.pop(delims);
    machine.pop(startpos);
    machine.pop(str);
    if (startpos >= str.size())
         throw std::runtime_error("STRCSPN: startpos out of range");
    int64_t pos = strcspn(str.c_str()+startpos, delims.c_str());
    machine.push(pos);
}


} // namespace rps

