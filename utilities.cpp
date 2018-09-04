#include <memory>
#include <cassert>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include "object.h"
#include "module.h"
#include "machine.h"

StringPtr MakeString()
{
    StringPtr sp;
    sp.reset(new String(""));
    return sp;
}

IntegerPtr MakeInteger()
{
    IntegerPtr ip;
    ip.reset(new Integer(0));
    return ip;
}

ProgramPtr MakeProgram()
{
    ProgramPtr pp;
    pp.reset(new Program());
    return pp;
}

ListPtr MakeList()
{
    ListPtr lp;
    lp.reset(new List());
    return lp;
}

MapPtr MakeMap()
{
    MapPtr mp;
    mp.reset(new Map());
    return mp;
}

ObjectPtr Clone(ObjectPtr optr)
{
    switch (optr->type)
    {
    case OBJECT_STRING:
        {
            StringPtr sp = MakeString();
            sp->value = ((String *)optr.get())->value;
            return sp;
        }
        break;
    case OBJECT_INTEGER:
        {
            IntegerPtr ip = MakeInteger();
            ip->value = ((Integer *)optr.get())->value;
            return ip;
        }
        break;
    case OBJECT_LIST:
        {
            ListPtr lp = MakeList();
            lp->items = ((List *)optr.get())->items;
            return lp;
        }
        break;
    case OBJECT_PROGRAM:
        {
            ProgramPtr pp = MakeProgram();
            *pp = *((Program *)optr.get());
            return pp;
        }
        break;
    default:
        assert(false);
        throw std::runtime_error("Clone: Unknown type");
        break;
    }
}

std::string ToStr(Machine& machine, ObjectPtr optr)
{
    switch (optr->type)
    {
    case OBJECT_STRING:
        return "\"" + ((String *)optr.get())->value + "\"";
    case OBJECT_INTEGER:
        return std::to_string(((Integer *)optr.get())->value);
    case OBJECT_COMMAND:
        return ((Command *)optr.get())->value;
    case OBJECT_LIST:
        {
            std::stringstream strm;
            List *lp = (List *)optr.get();
            strm << "[ ";
            size_t max = std::min(lp->items.size(), machine.list_maxcount);
            size_t count = 0;
            for (auto it = lp->items.begin(); count < max; ++it)
            {
                ++count;
                strm << ToStr(machine, *it);
                if (count < lp->items.size())
                    strm << ", ";
            }
            if (count < lp->items.size() )
            {
                strm << "...";
            }
            strm << " ]";
            return strm.str();
        }
        break;
    case OBJECT_MAP:
        {
            std::stringstream strm;
            Map *mp = (Map *)optr.get();
            strm << "{ ";
            size_t max = std::min(mp->items.size(), machine.list_maxcount);
            size_t count = 0;
            for (auto it = mp->items.begin(); count < max; ++it)
            {
                ++count;
                std::string k = ToStr(machine, it->first);
                std::string v = ToStr(machine, it->second);
                strm << k << ":" << v;
                if (count < mp->items.size())
                    strm << ", ";
            }
            if (count < mp->items.size())
            {
                strm << "...";
            }
            strm << " }";
            return strm.str();
        }
        break;
    case OBJECT_PROGRAM:
        {
            std::stringstream strm;
            Program *pp = (Program *)optr.get();
            strm << "<< ";
            if (pp->program.size() < 30)
            {
                for (ObjectPtr& op : pp->program)
                {
                    strm << ToStr(machine, op) << " ";
                }
            }
            else
            {
                strm << "...";
            }
            strm << " >>";
            return strm.str();
        }
        break;
    case OBJECT_IF:
        return "IF";
        break;
    default:
        std::cout << "=== ToStr: " << optr->token << std::endl;
        assert(false);
        throw std::runtime_error("Clone: Unknown type");
        break;
    }
}

std::string ToType(Machine&, ObjectPtr optr)
{
    switch (optr->type)
    {
    case OBJECT_STRING:
        return "S";
    case OBJECT_INTEGER:
        return "I";
    case OBJECT_COMMAND:
        return "C";
    case OBJECT_LIST:
        return "L";
    case OBJECT_MAP:
        return "M";
    case OBJECT_PROGRAM:
        return "P";
    default:
        assert(false);
        throw std::runtime_error("Clone: Unknown type");
        break;
    }
}
bool ToBool(ObjectPtr optr)
{
    switch(optr->type)
    {
    case OBJECT_STRING:
        return !((String *)optr.get())->value.empty();
    case OBJECT_INTEGER:
        return ((Integer *)optr.get())->value != 0;
    case OBJECT_LIST:
        return !((List *)optr.get())->items.empty();
    case OBJECT_PROGRAM:
        return false;
    case OBJECT_COMMAND:
        assert(false);
        break;
    }
}

