#include <vector>
#include <unordered_map>
#include <exception>
#include <iostream>
#include <sstream>
#include "token.h"
#include "object.h"
#include "module.h"
#include "machine.h"
#include "utilities.h"

void STO(Machine& machine)
{
    std::string s;
    ObjectPtr optr;
    if (machine.peek(0)->type != OBJECT_STRING)
    {
        std::stringstream strm;
        strm << "STO expected string, got " << ToStr(machine, machine.peek(0));
        throw std::runtime_error(strm.str().c_str());
    }
    machine.pop(s);
    std::string modname = machine.current_module_;
    std::string varname = s;
    size_t n = s.find_first_of('.');
    if (n != std::string::npos)
    {
        modname = s.substr(0, n);
        ++n;
        varname = s.substr(n);
    }
    machine.pop(optr);
    Module& module = machine.modules_[modname];
    module.variables_[varname] = optr;
}

void STOL(Machine& machine)
{
    std::string s;
    ObjectPtr optr;
    if (machine.stack_.size() < 2)
    {
        throw std::runtime_error("STOL: Requires string at L0 and object at L1");
    }
    if (machine.peek(0)->type != OBJECT_STRING)
    {
        std::stringstream strm;
        strm << "STOL expected string, got " << ToStr(machine, machine.peek(0));
        throw std::runtime_error(strm.str().c_str());
    }
    if (machine.current_program.get() == nullptr)
    {
        throw std::runtime_error("STOL: Requires current program context");
    }
    machine.pop(s);
    machine.pop(optr);
    machine.current_program->locals[s] = optr;
}

void RCL(Machine& machine, const std::string& name, ObjectPtr& out)
{
    std::string modname = machine.current_module_;
    std::string varname = name;
    size_t n = name.find_first_of('.');
    if (n != std::string::npos)
    {
        modname = name.substr(0, n);
        ++n;
        varname = name.substr(n);
    }
    auto itModule = machine.modules_.find(modname);
    if (itModule == machine.modules_.end())
    {
        std::stringstream strm;
        strm << "RCL Module " << modname << " not found";
        machine.push(name);
        throw std::runtime_error(strm.str().c_str());
    }
    auto itVar = itModule->second.variables_.find(varname);
    if (itVar == itModule->second.variables_.end())
    {
        machine.push(name);
        std::stringstream strm;
        strm << "RCL Variable " << modname << "." << varname << " not found";
        throw std::runtime_error(strm.str().c_str());
    }
    out = itVar->second;
}

void RCL(Machine& machine)
{
    std::string s;
    if (machine.peek(0)->type != OBJECT_STRING)
    {
        std::stringstream strm;
        strm << "RCL expected string, got " << ToStr(machine, machine.peek(0));
        throw std::runtime_error(strm.str().c_str());
    }
    ObjectPtr optr;
    machine.pop(s);
    RCL(machine, s, optr);
    machine.push(optr);
}

void RCLL(Machine& machine)
{
    std::string varname;
    ObjectPtr optr;
    if (machine.stack_.size() < 1)
    {
        throw std::runtime_error("RCCL: Requires string at L0");
    }
    if (machine.peek(0)->type != OBJECT_STRING)
    {
        std::stringstream strm;
        strm << "RCLL expected string, got " << ToStr(machine, machine.peek(0));
        throw std::runtime_error(strm.str().c_str());
    }
    if (machine.current_program.get() == nullptr)
    {
        throw std::runtime_error("RCCL: Requires current program context");
    }
    machine.pop(varname);
    auto itVar = machine.current_program->locals.find(varname);
    if (itVar == machine.current_program->locals.end())
    {
        machine.push(varname);
        std::stringstream strm;
        strm << "RCLL local Variable " << varname << " not found";
        throw std::runtime_error(strm.str().c_str());
    }
    machine.push(itVar->second);
}

void VARNAMES(Machine& machine)
{
    if (machine.stack_.size() < 1)
        throw std::runtime_error("VARNAMES requires an argument");
    std::string modname;
    machine.pop(modname);
    auto it = machine.modules_.find(modname);
    if (it == machine.modules_.end())
    {
        std::stringstream strm;
        strm << "VARNAMES: Module " << modname << " not found";
        throw std::runtime_error(strm.str().c_str());
    }
    auto itVars = it->second.variables_.begin();
    auto itEnd = it->second.variables_.end();
    ListPtr lp = MakeList();
    for (; itVars != itEnd; ++itVars)
    {
        StringPtr sp = MakeString();
        sp->value = modname + "." + itVars->first;
        lp->items.push_back(sp);
    }
    machine.push(lp);
}

void VARTYPES(Machine& machine)
{
    if (machine.stack_.size() < 1)
        throw std::runtime_error("VARTYPES requires an argument");
    std::string modname;
    machine.pop(modname);
    auto it = machine.modules_.find(modname);
    if (it == machine.modules_.end())
    {
        std::stringstream strm;
        strm << "TYPES: Module " << modname << " not found";
        throw std::runtime_error(strm.str().c_str());
    }
    auto itVars = it->second.variables_.begin();
    auto itEnd = it->second.variables_.end();
    ListPtr lp = MakeList();
    for (; itVars != itEnd; ++itVars)
    {
        StringPtr sp = MakeString();
        sp->value = ToType(machine, itVars->second) + ":" + modname + "." + itVars->first;
        lp->items.push_back(sp);
    }
    machine.push(lp);
}

