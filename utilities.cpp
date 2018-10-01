#include <memory>
#include <cassert>
#include <sstream>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <cstring>
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

NonePtr MakeNone()
{
    NonePtr np;
    np.reset(new None());
    return np;
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
    case OBJECT_NONE:
        {
            NonePtr np = MakeNone();
            return np;
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
        return ((String *)optr.get())->value;
    case OBJECT_INTEGER:
        return std::to_string(((Integer *)optr.get())->value);
    case OBJECT_COMMAND:
        return ((Command *)optr.get())->value;
    case OBJECT_LIST:
        {
            List *lp = (List *)optr.get();
            std::stringstream strm;
            strm << " [";
            for (auto it = lp->items.begin(); it != lp->items.end(); ++it)
            {
                if ((*it)->type == OBJECT_STRING)
                {
                    std::string s = ToStr(machine, *it);
                    if (s.find_first_of(' '))
                        strm << " \"" << s << "\"";
                    else
                        strm << " " << s;
                }
                else
                {
                    strm << " " << ToStr(machine, *it);
                }
            }
            strm << " ]";
            return strm.str();
        }
        break;
    case OBJECT_MAP:
        {
            Map *mp = (Map *)optr.get();
            std::stringstream strm;
            strm << " {";
            auto it = mp->items.begin();
            while (it != mp->items.end())
            {
                strm << " [";
                if (it->first->type == OBJECT_STRING)
                {
                    std::string s = ToStr(machine, it->first);
                    if (s.find_first_of(' '))
                        strm << " \"" << s << "\"";
                    else
                        strm << " " << s;
                }
                else
                {
                    strm << ToStr(machine, it->first);
                }
                if (it->second->type == OBJECT_STRING)
                {
                    std::string s = ToStr(machine, it->second);
                    if (s.find_first_of(' '))
                        strm << " \"" << s << "\"";
                    else
                        strm << " " << s;
                }
                else
                {
                    strm << " " << ToStr(machine, it->second);
                }
                strm << " ]";
                ++it;
            }
            strm << " }";
            return strm.str();
        }
        break;
    case OBJECT_PROGRAM:
        {
            Program *pp = (Program *)optr.get();
            std::stringstream strm;
            strm << " <<";
            for (ObjectPtr& op : pp->program)
            {
                if (op->type == OBJECT_STRING)
                {
                    std::string s = ToStr(machine, op);
                    if (s.find_first_of(' '))
                        strm << " \"" << s << "\"";
                    else
                        strm << " " << s;
                }
                else
                {
                    strm << " " << ToStr(machine, op);
                }
            }
            strm << " >>";
            return strm.str();
        }
        break;
    case OBJECT_IF:
        {
            If *pif = (If *)optr.get();
            std::stringstream strm;
            strm << " IF";
            for (ObjectPtr op : pif->cond)
            {
                if (op->type == OBJECT_STRING)
                {
                    std::string s = ToStr(machine, op);
                    if (s.find_first_of(' '))
                        strm << " " << "\"" << s << "\"";
                    else
                        strm << " " << s;
                }
                else
                {
                    strm << " " << ToStr(machine, op);
                }
            }
            strm << " THEN";
            for (ObjectPtr op : pif->then)
            {
                if (op->type == OBJECT_STRING)
                {
                    std::string s = ToStr(machine, op);
                    if (s.find_first_of(' '))
                        strm << " \"" << s << "\"";
                    else
                        strm << " " << s;
                }
                else
                {
                    strm << " " << ToStr(machine, op);
                }
            }
            if (pif->els.size())
            {
                strm << " ELSE";
                for (ObjectPtr op : pif->els)
                {
                    if (op->type == OBJECT_STRING)
                    {
                        std::string s = ToStr(machine, op);
                        if (s.find_first_of(' '))
                            strm << " \"" << s << "\"";
                        else
                            strm << " " << s;
                    }
                    else
                    {
                        strm << " " << ToStr(machine, op);
                    }
                }
            }
            strm << " ENDIF";
            return strm.str();
        }
        break;
    case OBJECT_FOR:
        {
            std::stringstream strm;
            strm << " FOR";
            For *pfor = (For *)optr.get();
            for (ObjectPtr op : pfor->program)
            {
                if (op->type == OBJECT_STRING)
                {
                    std::string s = ToStr(machine, op);
                    if (s.find_first_of(' '))
                        strm << " \"" << s << "\"";
                    else
                        strm << " " << s;
                }
                else
                {
                    strm << " " << ToStr(machine, op);
                }
            }
            strm << " ENDFOR";
            return strm.str();
        }
        break;
    case OBJECT_WHILE:
        {
            std::stringstream strm;
            strm << " WHILE";
            While *pwhile = (While *)optr.get();
            for (ObjectPtr op : pwhile->cond)
            {
                if (op->type == OBJECT_STRING)
                {
                    std::string s = ToStr(machine, op);
                    if (s.find_first_of(' '))
                        strm << " \"" << s << "\"";
                    else
                        strm << " " << s;
                }
                else
                {
                    strm << " " << ToStr(machine, op);
                }
            }
            strm << " REPEAT";
            for (ObjectPtr op : pwhile->program)
            {
                if (op->type == OBJECT_STRING)
                {
                    std::string s = ToStr(machine, op);
                    if (s.find_first_of(' '))
                        strm << " \"" << s << "\"";
                    else
                        strm << " " << s;
                }
                else
                {
                    strm << " " << ToStr(machine, op);
                }
            }
            strm << " ENDWHILE";
            return strm.str();
        }
        break;
    case OBJECT_TOKEN:
        {
            if (optr->token == TOKEN_EOL)
                return "\n";
            else
            {
                std::stringstream strm;
                strm << "TOKEN: " << optr->token;
                strm.str();
            }
        }
        break;
    case OBJECT_NONE:
        {
            return "None";
        }
        break;
    default:
        std::cout << "=== ToStr: " << optr->token << std::endl;
        assert(false);
        throw std::runtime_error("ToStr: Unknown type");
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
    case OBJECT_NONE:
        return "N";
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
    case OBJECT_NONE:
        return false;
    }
    assert(false);
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
    case OBJECT_NONE:
        throw std::runtime_error("Invalid None to Int conversion");
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

void GetArgs(Machine& machine, std::vector<std::string>& args)
{
    ObjectPtr optr;
    while (machine.stack_.size() > 0)
    {
        optr = machine.peek(0);
        if (optr->type != OBJECT_STRING)
            return;
        String *sp = (String *)optr.get();
        if ((strncmp(sp->value.c_str(), "--", 2) == 0))
        {
            args.push_back(sp->value);
            machine.pop(optr);
        }
        else
            return;
    }
}
