#include <memory>
#include <cassert>
#include <sstream>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include "object.h"
#include "module.h"
#include "machine.h"
#include "parser.h"

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

void ToStr(Machine& machine, ObjectPtr optr, std::stringstream& strm, bool view)
{
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
                ToStr(machine, *it, strm, view);
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
            ToStr(machine, it->first, strm, view);
            strm << ":";
            ToStr(machine, it->second, strm, view);
            ++it;
            for (; it != mp->items.end(); ++it)
            {
                strm << ", ";
                ToStr(machine, it->first, strm, view);
                strm << ":";
                ToStr(machine, it->second, strm, view);
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
                ToStr(machine, op, strm, view);
                strm << " ";
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
                ToStr(machine, op, strm, view);
            }
            strm << " THEN ";
            for (ObjectPtr op : pif->then)
                ToStr(machine, op, strm, view);
            if (pif->els.size())
            {
                strm << " ELSE ";
                for (ObjectPtr op : pif->els)
                    ToStr(machine, op, strm, view);
            }
        }
        break;
    case OBJECT_FOR:
        {
            strm << "FOR";
            For *pfor = (For *)optr.get();
            for (ObjectPtr op : pfor->program)
                ToStr(machine, op, strm, view);
        }
        break;
    case OBJECT_WHILE:
        {
            strm << "WHILE";
            While *pwhile = (While *)optr.get();
            for (ObjectPtr op : pwhile->cond)
                ToStr(machine, op, strm, view);
            strm << " REPEAT ";
            for (ObjectPtr op : pwhile->program)
                ToStr(machine, op, strm, view);
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
    ToStr(machine, optr, strm, false);
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


void split(const std::string& str, std::vector<std::string>& out, const std::string& delims, bool bCollapse)
{
    std::string s;
    for (auto it = str.begin(); it != str.end(); ++it)
    {
        if (delims.find_first_of(*it) != std::string::npos)
        {
            out.push_back(s);
            s = "";
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
        out.push_back(s);
    }
}

void Import(Machine& machine, Parser& parser, const std::string& modname)
{
    const char *rps_path = getenv("RPS_PATH");
    std::vector<std::string> dirs;
    dirs.push_back(".");
    if (rps_path)
        split(rps_path, dirs, ":", false);

    std::string filename;
    std::ifstream ifs;
    for (auto& dir : dirs)
    {
        filename = dir + "/" + modname + ".rps";
        ifs.open(filename);
        if (ifs.is_open())
            break;
    }
    if (!ifs.is_open())
    {
        std::stringstream strm;
        strm << "import: cannot find " << modname << ".rps";
        throw std::runtime_error(strm.str());
    }
    Source srcImport(ifs);
    std::string savename = machine.current_module_;
    machine.current_module_ = modname;
    machine.CreateModule(modname);
    parser.Parse(machine, srcImport);
    machine.current_module_ = savename;
}
