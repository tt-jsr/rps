#include <memory>
#include <cassert>
#include <sstream>
#include "object.h"

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

std::string ToStr(ObjectPtr& optr)
{
    switch (optr->type)
    {
    case OBJECT_STRING:
        return ((String *)optr.get())->value;
    case OBJECT_INTEGER:
        return std::to_string(((Integer *)optr.get())->value);
    case OBJECT_COMMAND:
        return ((Command *)optr.get())->value;
    case OBJECT_LIST:
        {
            std::stringstream strm;
            List *lp = (List *)optr.get();
            strm << "[ ";
            if (lp->items.size() < 30)
            {
                for (ObjectPtr& op : lp->items)
                {
                    strm << ToStr(op) << ", ";
                }
            }
            else
            {
                strm << "...";
            }
            strm << " ]";
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
                    strm << ToStr(op) << " ";
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
    default:
        assert(false);
        throw std::runtime_error("Clone: Unknown type");
        break;
    }
}
