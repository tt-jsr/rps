#include <vector>
#include <unordered_map>
#include <exception>
#include <iostream>
#include <sstream>
#include <cassert>
#include <memory>
#include <algorithm>
#include "token.h"
#include "object.h"
#include "module.h"
#include "machine.h"
#include "commands.h"
#include "utilities.h"

namespace rps
{

bool bInterrupt = false;

Machine::Machine()
{
    SetProperty("viewwidth", 120);
    SetProperty("debug", 0);
    SetProperty("help", 0);
}

void Machine::CreateModule(const std::string& name)
{
    Module mod;
    mod.module_name_ = name;
    modules_.emplace(name, std::move(mod));
}

void Machine::push(ObjectPtr& optr)
{
    int64_t debug = GetProperty("debug", 0);
    assert(optr->type != OBJECT_COMMAND);
    if (debug)
        std::cout << "push: " << ToStr(*this, optr) << std::endl;
    stack_.push_back(optr);
}

void Machine::pop()
{
    if (stack_.empty())
        throw std::runtime_error("stack underflow");
    stack_.pop_back();
}

void Machine::pop(ObjectPtr& optr)
{
    int64_t debug = GetProperty("debug", 0);
    if (stack_.empty())
        throw std::runtime_error("stack underflow");
    optr = stack_.back();
    if (debug)
        std::cout << "pop: " << ToStr(*this, optr) << std::endl;
    stack_.pop_back();
}

void Machine::pop(int64_t& v)
{
    if (stack_.empty())
        throw std::runtime_error("stack underflow");
    if (peek(0)->type != OBJECT_INTEGER)
    {
        std::stringstream strm;
        strm << "pop: Expected integer, got: " << ToStr(*this, peek(0));
        throw std::runtime_error(strm.str().c_str());
    }
    ObjectPtr optr;
    pop(optr);
    Integer *ip = (Integer *)optr.get();
    v = ip->value;
}

void Machine::push(int64_t v)
{
    ObjectPtr optr;
    optr.reset(new Integer(v));
    push(optr);
}

void Machine::pop(ListPtr& lp)
{
    if (stack_.empty())
        throw std::runtime_error("stack underflow");
    if (peek(0)->type != OBJECT_LIST)
    {
        std::stringstream strm;
        strm << "pop: Expected list, got: " << ToStr(*this, peek());
        throw std::runtime_error(strm.str().c_str());
    }
    ObjectPtr optr;
    pop(optr);
    lp = std::static_pointer_cast<List>(optr);
}

void Machine::push(ListPtr& lp)
{
    ObjectPtr op = lp;
    push(op);
}

void Machine::pop(MapPtr& mp)
{
    if (stack_.empty())
        throw std::runtime_error("stack underflow");
    if (peek(0)->type != OBJECT_MAP)
    {
        std::stringstream strm;
        strm << "pop: Expected map, got: " << ToStr(*this, peek());
        throw std::runtime_error(strm.str().c_str());
    }
    ObjectPtr optr;
    pop(optr);
    mp = std::static_pointer_cast<Map>(optr);
}

void Machine::push(MapPtr& mp)
{
    ObjectPtr op = mp;
    push(op);
}

void Machine::pop(ProgramPtr& pp)
{
    if (stack_.empty())
        throw std::runtime_error("stack underflow");
    if (peek(0)->type != OBJECT_PROGRAM)
    {
        std::stringstream strm;
        strm << "pop: Expected program, got: " << ToStr(*this, peek());
        throw std::runtime_error(strm.str().c_str());
    }
    ObjectPtr optr;
    pop(optr);
    pp = std::static_pointer_cast<Program>(optr);
}

void Machine::push(ProgramPtr& pp)
{
    ObjectPtr op = pp;
    push(op);
}

void Machine::pop(std::string& v)
{
    if (stack_.empty())
        throw std::runtime_error("stack underflow");
    if (peek(0)->type != OBJECT_STRING)
    {
        std::stringstream strm;
        strm << "pop: Expected string, got: " << ToStr(*this, peek());
        throw std::runtime_error(strm.str().c_str());
    }
    ObjectPtr optr;
    pop(optr);
    String *sp = (String *)optr.get();
    v = sp->get();
}

void Machine::push(const std::string& v)
{
    ObjectPtr optr;
    optr.reset(new String(v));
    stack_.push_back(optr);
}

ObjectPtr& Machine::peek()
{
    if (stack_.empty())
        throw std::runtime_error("stack underflow");
    return stack_.back();
}

ObjectPtr& Machine::peek(size_t n)
{
    ++n;
    if (stack_.size() < n)
        throw std::runtime_error("stack underflow");
    return stack_[stack_.size()-n];
}

void Machine::SetProperty(const std::string& name, int64_t n)
{
    IntegerPtr ip = MakeInteger();
    ip->value = n;
    properties[name] = ip;
}

void Machine::SetProperty(const std::string& name, const std::string& value)
{
    StringPtr sp = MakeString();
    sp->set(value);
    properties[name] = sp;
}

void Machine::SetProperty(const std::string& name, ObjectPtr optr)
{
    properties[name] = optr;
}

int64_t Machine::GetProperty(const std::string& name, int64_t def)
{
    auto it = properties.find(name);
    if (it == properties.end())
        return def;
    ObjectPtr optr = it->second;
    if (optr->type != OBJECT_INTEGER)
        return def;
    return ((Integer *)optr.get())->value;
}

std::string Machine::GetProperty(const std::string& name, const std::string& def)
{
    auto it = properties.find(name);
    if (it == properties.end())
        return def;
    ObjectPtr optr = it->second;
    if (optr->type != OBJECT_STRING)
        return def;
    return ((String *)optr.get())->get();
}

ObjectPtr Machine::GetProperty(const std::string& name)
{
    auto it = properties.find(name);
    if (it == properties.end())
        return ObjectPtr();
    return it->second;
}

void Machine::AddAlias(const std::string& name, const std::vector<std::string>& vec)
{
    aliases[name] = vec;
}

std::vector<std::string> * Machine::GetAlias(const std::string& name)
{
    auto it = aliases.find(name);
    if (it == aliases.end())
        return nullptr;
    return &it->second;
}


std::ostream& Machine::helpstrm()
{
    if (hstrm.str().size())
        hstrm << std::endl;
    return hstrm;
}
std::string GetFunctionSynopsis(Machine& machine, const std::string& cmd)
{
    auto it = machine.commands.find(cmd);
    if (it != machine.commands.end())
    {
        machine.SetProperty("help", 1);
        machine.hstrm.str("");
        (*it->second->funcptr)(machine);
        machine.SetProperty("help", 0);
        std::string s = machine.hstrm.str();
        size_t pos = s.find_first_of('\n');
        if (pos != std::string::npos)
            return s.substr(0, pos);
    }
    return "";
}

void ShowCategory(Machine& machine, const std::string& cat)
{
    auto it = machine.categories.find(cat);
    if (it != machine.categories.end())
    {
        std::cout << cat << std::endl;
        for (const std::string& cmd : it->second)
        {
            std::cout << "    " << GetFunctionSynopsis(machine, cmd) << std::endl;
        }
    }
}

void ShowHelp(Machine& machine, CommandPtr cmd)
{
    if (cmd->funcptr == nullptr)
    {
        std::cout << "Help not available for Registered programs" << std::endl;
        return;
    }
    machine.SetProperty("help", 1);
    machine.hstrm.str("");
    (*cmd->funcptr)(machine);
    machine.SetProperty("help", 0);
    std::cout << std::endl << machine.hstrm.str() << std::endl;
}

void HELP(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "HELP: HELP system";
        machine.helpstrm() << "HELP =>";
        return;
    }
    while (true)
    {
        for (auto& pr : machine.categories)
        {
            assert(pr.first.size() <= 19);   // This is the width of the longest category name
            std::string pad(19-pr.first.size(), ' ');
            std::cout << pad << pr.first << ": ";
            for (auto& c : pr.second)
            {
                std::cout << c << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl << "Command or Category: " << std::flush;
        std::string cmd;
        std::getline(std::cin, cmd);
        if (cmd.size() == 0)
            return;
        auto it = machine.commands.find(cmd);
        if (it != machine.commands.end())
        {
            ShowHelp(machine, it->second);
            std::getline(std::cin, cmd);
        }
        else
        {
            ShowCategory(machine, cmd);
            std::getline(std::cin, cmd);
        }
    }
}

void throw_required(Machine& machine, const char *f, int level, ObjectType t)
{
    if (machine.peek(level)->type != t)
    {
        std::stringstream strm;
        strm << f << ": Level " << level << " required to be a " 
            << ObjectNames[t] << " got " << ObjectNames[machine.peek(level)->type];
        throw std::runtime_error(strm.str().c_str());
    }
}

void stack_required(Machine& machine, const char *f, int depth)
{
    if (machine.stack_.size() < depth-1)
    {
        std::stringstream strm;
        strm << f << ": Requires " << depth << " stacklevels";
        throw std::runtime_error(strm.str().c_str());
    }
}


void AddCommand(Machine& machine, const std::string& name, void (*funcptr)(Machine&))
{
    CommandPtr cp;
    cp.reset(new Command(name, funcptr));
    machine.commands.emplace(name, cp);

    cp.reset(new Command(name+"?", funcptr));
    machine.commands.emplace(name+"?", cp);
}

void AddCommand(Machine& machine, const std::string& name, ProgramPtr pptr)
{
    CommandPtr cp;
    cp.reset(new Command(name, nullptr));
    cp->program = pptr;
    machine.commands.emplace(name, cp);

    Category(machine, "RegisteredPrograms", name);
}

void RemoveCommand(Machine& machine, const std::string& name)
{
    machine.commands.erase(name);

    std::set<std::string>& st = machine.categories["RegisteredPrograms"];
    auto it = st.find(name);
    if (it != st.end())
        st.erase(it);
}

void Category(Machine& machine, const std::string& cat, const std::string& name)
{
    std::set<std::string>& st = machine.categories[cat];
    st.insert(name);
}

} // namespace rps


