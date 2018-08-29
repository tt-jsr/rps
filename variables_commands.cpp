#include <vector>
#include <unordered_map>
#include <exception>
#include <iostream>
#include <sstream>
#include "token.h"
#include "object.h"
#include "module.h"
#include "machine.h"

void STO(Machine& machine)
{
    std::string s;
    ObjectPtr optr;
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
    auto itModule = machine.modules_.find(modname);
    if (itModule == machine.modules_.end())
    {
        std::stringstream strm;
        strm << "STO Module " << modname << " not found";
        machine.push(optr);
        machine.push(s);
        throw std::runtime_error(strm.str().c_str());
    }
    itModule->second.variables_.emplace(varname, optr);
}

void RCL(Machine& machine)
{
    std::string s;
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
    auto itModule = machine.modules_.find(modname);
    if (itModule == machine.modules_.end())
    {
        std::stringstream strm;
        strm << "RCL Module " << modname << " not found";
        machine.push(s);
        throw std::runtime_error(strm.str().c_str());
    }
    auto itVar = itModule->second.variables_.find(varname);
    if (itVar == itModule->second.variables_.end())
    {
        machine.push(s);
        std::stringstream strm;
        strm << "RCL Variable " << modname << "." << varname << " not found";
        throw std::runtime_error(strm.str().c_str());
    }
    machine.push(itVar->second);
}

