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
#include "commands.h"

void STO(Machine& machine)
{
    std::string s;
    ObjectPtr optr;
    stack_required(machine, "STO", 2);
    throw_required(machine, "STO", 0, OBJECT_STRING);

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
    stack_required(machine, "STOL", 2);
    throw_required(machine, "STOL", 0, OBJECT_STRING);
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
    stack_required(machine, "RCL", 1);
    throw_required(machine, "RCL", 0, OBJECT_STRING);

    ObjectPtr optr;
    machine.pop(s);
    RCL(machine, s, optr);
    machine.push(optr);
}

void RCLL(Machine& machine)
{
    stack_required(machine, "RCLL", 1);
    throw_required(machine, "RCLL", 0, OBJECT_STRING);

    std::string varname;
    ObjectPtr optr;
    machine.pop(varname);
    RCLL(machine, varname, optr);
    machine.push(optr);
}

void RCLL(Machine& machine, const std::string& name, ObjectPtr& out)
{
    if (machine.current_program.get() == nullptr)
    {
        throw std::runtime_error("RCCL: Requires current program context");
    }
    auto itVar = machine.current_program->locals.find(name);
    if (itVar == machine.current_program->locals.end())
    {
        std::stringstream strm;
        strm << "RCLL local Variable " << name << " not found";
        throw std::runtime_error(strm.str().c_str());
    }
    out = itVar->second;
}

void VARNAMES(Machine& machine)
{
    stack_required(machine, "VARNAMES", 1);
    throw_required(machine, "VARNAMES", 0, OBJECT_STRING);
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
    stack_required(machine, "VARTYPES", 1);
    throw_required(machine, "VARTYPES", 0, OBJECT_STRING);

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

