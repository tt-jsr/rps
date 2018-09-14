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
    case OBJECT_MAP:
        {
            MapPtr mp = MakeMap();
            *mp = *((Map *)optr.get());
            return mp;
        }
        break;
    default:
        assert(false);
        throw std::runtime_error("Clone: Unknown type");
        break;
    }
}

void ToStr(Machine& machine, ObjectPtr optr, std::stringstream& strm, size_t maxwidth, bool view)
{
    if (strm.str().size() > maxwidth)
    {
        strm << "...";
        return;
    }
    switch (optr->type)
    {
    case OBJECT_STRING:
        if(view)
            strm << "\"" << ((String *)optr.get())->value << "\"";
        else
            strm << ((String *)optr.get())->value;
        break;
    case OBJECT_INTEGER:
        strm << std::to_string(((Integer *)optr.get())->value);
        break;
    case OBJECT_COMMAND:
        strm << ((Command *)optr.get())->value;
        break;
    case OBJECT_LIST:
        {
            List *lp = (List *)optr.get();
            strm << "[ ";
            for (auto it = lp->items.begin(); it != lp->items.end(); ++it)
            {
                ToStr(machine, *it, strm, maxwidth, view);
                if (strm.str().size() >= maxwidth)
                {
                    strm << "...";
                    break;
                }
                if ((it+1) != lp->items.end())
                    strm << ", ";
            }
            strm << " ]";
        }
        break;
    case OBJECT_MAP:
        {
            Map *mp = (Map *)optr.get();
            strm << "{ ";
            auto it = mp->items.begin();
            ToStr(machine, it->first, strm, maxwidth, view);
            strm << ":";
            ToStr(machine, it->second, strm, maxwidth, view);
            ++it;
            for (; it != mp->items.end(); ++it)
            {
                strm << ", ";
                ToStr(machine, it->first, strm, maxwidth, view);
                strm << ":";
                ToStr(machine, it->second, strm, maxwidth, view);
                if (strm.str().size() >= maxwidth)
                {
                    strm << "...";
                    break;
                }
            }
            strm << " }";
        }
        break;
    case OBJECT_PROGRAM:
        {
            Program *pp = (Program *)optr.get();
            strm << "<< ";
            for (ObjectPtr& op : pp->program)
            {
                ToStr(machine, op, strm, maxwidth, view);
                strm << " ";
                if (strm.str().size() >= maxwidth, view)
                {
                    strm << "...";
                    break;
                }
            }
            strm << " >>";
        }
        break;
    case OBJECT_IF:
        {
            If *pif = (If *)optr.get();
            strm << "IF";
            for (ObjectPtr op : pif->cond)
            {
                ToStr(machine, op, strm, maxwidth, view);
            }
            strm << " THEN ";
            for (ObjectPtr op : pif->then)
                ToStr(machine, op, strm, maxwidth, view);
            if (pif->els.size())
            {
                strm << " ELSE ";
                for (ObjectPtr op : pif->els)
                    ToStr(machine, op, strm, maxwidth, view);
            }
        }
        break;
    case OBJECT_FOR:
        {
            strm << "FOR";
            For *pfor = (For *)optr.get();
            for (ObjectPtr op : pfor->program)
                ToStr(machine, op, strm, maxwidth, view);
        }
        break;
    case OBJECT_WHILE:
        {
            strm << "WHILE";
            While *pwhile = (While *)optr.get();
            for (ObjectPtr op : pwhile->cond)
                ToStr(machine, op, strm, maxwidth, view);
            strm << " REPEAT ";
            for (ObjectPtr op : pwhile->program)
                ToStr(machine, op, strm, maxwidth, view);
        }
        break;
    case OBJECT_TOKEN:
        {
            if (optr->token == TOKEN_EOL)
                strm << "<EOL>";
            else
            {
                strm << "TOKEN: " << optr->token;
            }
        }
        break;
    default:
        std::cout << "=== ToStr: " << optr->token << std::endl;
        assert(false);
        throw std::runtime_error("ToStr: Unknown type");
        break;
    }
}

std::string ToStr(Machine& machine, ObjectPtr optr)
{
    std::stringstream strm;
    ToStr(machine, optr, strm, machine.maxwidth, false);
    return strm.str();
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

bool ToBool(Machine&, ObjectPtr optr)
{
    switch(optr->type)
    {
    case OBJECT_STRING:
        return !((String *)optr.get())->value.empty();
    case OBJECT_INTEGER:
        return ((Integer *)optr.get())->value != 0;
    case OBJECT_LIST:
        return !((List *)optr.get())->items.empty();
    case OBJECT_MAP:
        return !((Map *)optr.get())->items.empty();
    case OBJECT_PROGRAM:
        return false;
    case OBJECT_COMMAND:
        assert(false);
        break;
    }
}

int64_t ToInt(Machine& machine, ObjectPtr optr)
{
    switch(optr->type)
    {
    case OBJECT_STRING:
        {
            String *p =  (String *)optr.get();
            return std::stoll(p->value);
        }
    case OBJECT_INTEGER:
        return ((Integer *)optr.get())->value;
    case OBJECT_LIST:
        throw std::runtime_error("Invalid List to Int conversion");
    case OBJECT_MAP:
        throw std::runtime_error("Invalid Map to Int conversion");
    case OBJECT_PROGRAM:
        throw std::runtime_error("Invalid Program to Int conversion");
    case OBJECT_COMMAND:
        assert(false);
        break;
    }
}

