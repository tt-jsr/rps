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

namespace rps
{


ObjectPtr find_local(Machine& machine, const std::string& name)
{
    ProgramPtr pp = machine.current_program;
    ObjectPtr optr;

    auto itVar = pp->pLocals->find(name);
    if (itVar == pp->pLocals->end())
    {
        return optr;
    }
    return itVar->second;
}

void STO(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "STO: Store object";
        machine.helpstrm() << "obj \"name\" STO =>";
        machine.helpstrm() << "obj \"namespace.name\" STO =>";
        machine.helpstrm() << "Store an object in the given variable name.";
        machine.helpstrm() << "By default objects are stored in the current namespace which";
        machine.helpstrm() << "is usually the module name or set by SETNS";
        return;
    }

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
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "STOL: Store object into local storage";
        machine.helpstrm() << "obj \"name\" STOL =>";
        machine.helpstrm() << "obj \"name\" STOL* => obj";
        machine.helpstrm() << "Store an object in the given variable name in the local context.";
        machine.helpstrm() << "Local variables are only available in the current program.";
        machine.helpstrm() << "Programs defined within a program have access to local variables of the";
        machine.helpstrm() << "enclosing program";
        return;
    }

    std::string name;
    ObjectPtr optr;
    stack_required(machine, "STOL", 2);
    throw_required(machine, "STOL", 0, OBJECT_STRING);
    if (machine.current_program.get() == nullptr)
    {
        throw std::runtime_error("STOL: Requires current program context");
    }
    machine.pop(name);
    machine.pop(optr);

    ProgramPtr pp = machine.current_program;

    while (pp)
    {
        assert(pp->pLocals);
        auto itVar = pp->pLocals->find(name);
        if (itVar != pp->pLocals->end())
        {
            (*pp->pLocals)[name] = optr;
            return;
        }
        pp = pp->enclosingProgram;
    }
    (*machine.current_program->pLocals)[name] = optr;
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
        throw std::runtime_error(strm.str().c_str());
    }
    auto itVar = itModule->second.variables_.find(varname);
    if (itVar == itModule->second.variables_.end())
    {
        std::stringstream strm;
        strm << "RCL Variable " << modname << "." << varname << " not found";
        throw std::runtime_error(strm.str().c_str());
    }
    out = itVar->second;
}

void RCL(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "RCL: Recall object";
        machine.helpstrm() << "\"name\" RCL => obj";
        machine.helpstrm() << "\"namespace.name\" RCL => obj";
        machine.helpstrm() << "Recall an object from the given variable name.";
        machine.helpstrm() << "By default objects are recalled from the current namespace which";
        machine.helpstrm() << "is usually the module name or set by SETNS";
        machine.helpstrm() << "See also RCLL, RCLA";
        return;
    }

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
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "RCLL: Recall a local object";
        machine.helpstrm() << "\"name\" RCLL =>";
        machine.helpstrm() << "Local variables can be recalled from the current program";
        machine.helpstrm() << "or local variables of enclosing programs";
        machine.helpstrm() << "See also RCL, RCLA";
        return;
    }

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

    ProgramPtr pp = machine.current_program;

    while (pp)
    {
        auto itVar = pp->pLocals->find(name);
        if (itVar != pp->pLocals->end())
        {
            out = itVar->second;
            return;
        }
        pp = pp->enclosingProgram;
    }
    std::stringstream strm;
    strm << "RCLL local Variable " << name << " not found";
    throw std::runtime_error(strm.str().c_str());
}

void RCLA(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "RCLA: Recall an object, first look in local storage, then global";
        machine.helpstrm() << "\"name\" RCLA => obj";
        machine.helpstrm() << "Local variables can be recalled from the current program";
        machine.helpstrm() << "or local variables of enclosing programs";
        machine.helpstrm() << "\'%\' is a synonym for RCLA";
        return;
    }

    stack_required(machine, "RCLA", 1);
    throw_required(machine, "RCLA", 0, OBJECT_STRING);

    std::string varname;
    ObjectPtr optr;
    machine.pop(varname);
    try
    {
        RCLL(machine, varname, optr);
        machine.push(optr);
    }
    catch(std::exception&)
    {
        RCL(machine, varname, optr);
        machine.push(optr);
    }
}

void VARS(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "VARS: Print variables and thier contents";
        machine.helpstrm() << "\"namespace\" VARS =>";
        return;
    }

    stack_required(machine, "VARS", 1);
    throw_required(machine, "VARS", 0, OBJECT_STRING);
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
        std::cout << modname << "." << itVars->first 
            << "[" << ToType(machine,itVars->second )
            << "] = " ;
        std::string s = ToStr(machine, itVars->second);
        for (size_t idx = 0; s[idx] && idx < 80; ++idx)
        {
            if (s[idx] == '\n')
                std::cout << "\\n";
            else
                std::cout << s[idx];
        }
        std::cout << std::endl;
    }
}

void VARNAMES(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "VARNAMES: List variables";
        machine.helpstrm() << "\"namespace\" VARNAMES => [list]";
        return;
    }

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
        sp->set(modname + "." + itVars->first);
        lp->items.push_back(sp);
    }
    machine.push(lp);
}

void VARTYPES(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "VARTYPES: List variable types";
        machine.helpstrm() << "\"namespace\" VARTYPES => [list]";
        return;
    }

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
        sp->set(ToType(machine, itVars->second) + ":" + modname + "." + itVars->first);
        lp->items.push_back(sp);
    }
    machine.push(lp);
}

void REGISTER(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "REGISTER: Register a user defined program";
        machine.helpstrm() << "<<prog>> \"name\" REGISTER => ";
        machine.helpstrm() << "Registering a program allows the user invoke the program by name";
        machine.helpstrm() << "rather than having to issue a CALL statement.";
        machine.helpstrm() << "name should always be in quotes to force interpretation as a string,";
        machine.helpstrm() << "otherwise calling REGISTER again could invoke the command rather then setting it.";
        return;
    }
    stack_required(machine, "REGISTER", 2);
    throw_required(machine, "REGISTER", 0, OBJECT_STRING);
    throw_required(machine, "REGISTER", 1, OBJECT_PROGRAM);
    ObjectPtr prog;
    std::string name;

    machine.pop(name);
    machine.pop(prog);
    ProgramPtr pptr = std::static_pointer_cast<Program>(prog);

    AddCommand(machine, name, pptr);
}

void UNREGISTER(Machine& machine)
{
    if (machine.GetProperty("help", 0))
    {
        machine.helpstrm() << "UNREGISTER: Unregister a user defined program";
        machine.helpstrm() << "\"name\" UNREGISTER => ";
        machine.helpstrm() << "Unregisters a previous registered program";
        return;
    }
    stack_required(machine, "UNREGISTER", 1);
    throw_required(machine, "UNREGISTER", 0, OBJECT_STRING);
    std::string name;

    machine.pop(name);

    RemoveCommand(machine, name);
}


} // namespace rps

