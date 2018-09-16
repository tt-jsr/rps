#include <vector>
#include <unordered_map>
#include <exception>
#include <iostream>
#include <sstream>
#include <cassert>
#include <memory>
#include <unistd.h>
#include "token.h"
#include "object.h"
#include "module.h"
#include "machine.h"
#include "commands.h"
#include "utilities.h"

Machine::Machine()
: maxwidth(120)
, debug(false) 
, help(false)
{
}

void Machine::CreateModule(const std::string& name)
{
    Module mod;
    mod.module_name_ = name;
    modules_.emplace(name, std::move(mod));
}

void Machine::push(ObjectPtr& optr)
{
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
    v = sp->value;
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

/****************************************************************/
void Execute(Machine& machine, std::vector<ObjectPtr>& vec)
{
    for (ObjectPtr& op : vec)
    {
        //std::cout << "=== machine::Execute vecsize: " << vec.size() << std::endl;
        Execute(machine, op);
    }
}

void Execute(Machine& machine, ObjectPtr optr)
{
    //std::cout << "=== machine:::EVAL str: " << ToStr(machine, optr) << std::endl;
    switch(optr->type)
    {
    case OBJECT_STRING:
        machine.push(optr);
        break;
    case OBJECT_INTEGER:
        machine.push(optr);
        break;
    case OBJECT_LIST:
        machine.push(optr);
        break;
    case OBJECT_PROGRAM:
        machine.push(optr);
        break;
    case OBJECT_IF:
        {
            ObjectPtr ob;
            If *p = (If *)optr.get();
            Execute(machine, p->cond);
            machine.pop(ob);
            if (ToBool(machine, ob))
                Execute(machine, p->then);
            else
                Execute(machine, p->els);
        }
        break;
    case OBJECT_FOR:
        {
            ObjectPtr ob;
            For *p = (For *)optr.get();
            try
            {
                if (machine.stack_.size() == 0)
                    std::cout << "FOR requires list or map at L0" << std::endl;
                else if (machine.peek(0)->type == OBJECT_LIST)
                {
                    ListPtr lp;
                    machine.pop(lp);
                    for (ObjectPtr op : lp->items)
                    {
                        machine.push(op);
                        Execute(machine, p->program);
                    }
                }
                else if (machine.peek(0)->type == OBJECT_MAP)
                {
                    MapPtr mp;
                    machine.pop(mp);
                    for (auto& pr : mp->items)
                    {
                        ListPtr lp = MakeList();
                        lp->items.push_back(pr.first);
                        lp->items.push_back(pr.second);
                        machine.push(lp);
                        Execute(machine, p->program);
                    }
                }
            }
            catch (std::exception& e)
            {
                std::cout << e.what() << std::endl;
            }
        }
        break;
    case OBJECT_WHILE:
        {
            ObjectPtr ob;
            While *p = (While *)optr.get();
            try
            {
                while (true)
                {
                    Execute(machine, p->cond);
                    ObjectPtr optr;
                    machine.pop(optr);
                    bool b = ToBool(machine, optr);
                    if (b) 
                    {
                        Execute(machine, p->program);
                    }
                    else
                        break;
                }
            }
            catch (std::exception& e)
            {
                std::cout << e.what() << std::endl;
            }
        }
        break;
    case OBJECT_COMMAND:
        {
            Command *pCommand = (Command *)optr.get();
            (*pCommand->funcptr)(machine);
        }
        break;
    case OBJECT_TOKEN:
        break;
    default:
        std::cout << "=== EVAL: " << ToStr(machine, optr) << std::endl;
        assert(false);
        break;
    }
}

void Execute(Machine& machine)
{
    ObjectPtr optr;
    machine.pop(optr);
    Execute(machine, optr);
}

void EVAL(Machine& machine, ObjectPtr optr)
{
    //std::cout << "=== machine:::EVAL str: " << ToStr(machine, optr) << std::endl;
    switch(optr->type)
    {
    case OBJECT_STRING:
        machine.push(optr);
        break;
    case OBJECT_INTEGER:
        machine.push(optr);
        break;
    case OBJECT_LIST:
        machine.push(optr);
        break;
    case OBJECT_PROGRAM:
        {
            ProgramPtr prev_program = machine.current_program;
            machine.current_program = std::static_pointer_cast<Program>(optr);
            std::string prev_module = machine.current_module_;
            machine.current_module_ = machine.current_program->module_name;
            try
            {
                std::unordered_map<std::string, ObjectPtr> locals;

                machine.current_program->pLocals = &locals;
                Execute(machine, machine.current_program->program);
                machine.current_program->pLocals = nullptr;
                machine.current_module_ = prev_module;
                machine.current_program = prev_program;
            }
            catch (std::exception& e)
            {
                machine.current_program->pLocals = nullptr;
                machine.current_module_ = prev_module;
                machine.current_program = prev_program;
                throw;
            }
        }
        break;
    case OBJECT_TOKEN:
        break;
    default:
        std::cout << "=== EVAL: " << ToStr(machine, optr) << std::endl;
        assert(false);
        break;
    }
}

void EVAL(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "EVAL: Evaluate an object" << std::endl;
        std::cout << "obj EVAL => ..." << std::endl;
        return;
    }
    ObjectPtr optr;
    machine.pop(optr);
    EVAL(machine, optr);
}

void CALL(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "CALL: Call a program" << std::endl;
        std::cout << "\"name\" CALL => ..." << std::endl;
        std::cout << "Equivilent to name RCL EVAL" << std::endl;
        return;
    }
    RCL(machine);
    EVAL(machine);
}

void NAMESPACES(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "NAMESPACES: List namespaces" << std::endl;
        std::cout << "NAMESPACES => [list]" << std::endl;
        return;
    }
    ListPtr lp = MakeList();
    for (auto& pr : machine.modules_)
    {
        StringPtr sp = MakeString();
        sp->value = pr.first;
        lp->items.push_back(sp);
    }
    machine.push(lp);
}

void CLONE(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "CLONE: Clone the object at L0" << std::endl;
        std::cout << "obj CLONE => obj" << std::endl;
        return;
    }
    stack_required(machine, "CLONE", 1);

    ObjectPtr optr = Clone(machine.stack_[machine.stack_.size()-1]);
    machine.push(optr);
}

void SETNS(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "SETNS: Create or change namespace" << std::endl;
        std::cout << "\"namespace\" SETNS =>" << std::endl;
        return;
    }
    stack_required(machine, "SETNS", 1);
    throw_required(machine, "SETNS", 0, OBJECT_STRING);

    std::string s;
    machine.pop(s);
    machine.CreateModule(s);
    machine.current_module_ = s;
}

void GETNS(Machine& machine)
{ 
    if (machine.help)
    {
        std::cout << "GETNS: Push current namespace" << std::endl;
        std::cout << "GETNS => \"namespace\"" << std::endl;
        return;
    }
    machine.push(machine.current_module_);
}

void CD(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "CD: Change current dir" << std::endl;
        std::cout << "\"dir\" CD =>" << std::endl;
        return;
    }
    stack_required(machine, "CD", 1);
    throw_required(machine, "CD", 0, OBJECT_STRING);

    std::string dir;
    machine.pop(dir);
    chdir(dir.c_str());
}

void HELP(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "HELP: HELP system" << std::endl;
        std::cout << "HELP =>" << std::endl;
        return;
    }
    while (true)
    {
        std::cout << "Stack commands" << std::endl;
        std::cout << "   DROP DROPN CLRSTK SWAP DUP PICK ROLL VIEW DEPTH" << std::endl;
        std::cout << "" << std::endl;
        std::cout << "Math commands" << std::endl;
        std::cout << "   ADD SUB MUL DIV INC DEC" << std::endl;
        std::cout << "" << std::endl;
        std::cout << "Variable management" << std::endl;
        std::cout << "   STO RCL STOL RCLL VARNAMES VARTYPES" << std::endl;
        std::cout << "" << std::endl;
        std::cout << "Controlcommands" << std::endl;
        std::cout << "   IFT IFTE TRYCATCH " << std::endl;
        std::cout << "" << std::endl;
        std::cout << "Logical operators" << std::endl;
        std::cout << "   EQ NEQ LT LTEQ GT GTEQ NOT AND OR" << std::endl;
        std::cout << "" << std::endl;
        std::cout << "List commands" << std::endl;
        std::cout << "   APPEND GET LIST-INSERT ERASE CLEAR SIZE FIRST SECOND TOLIST FROMLIST CREATELIST " << std::endl;
        std::cout << "" << std::endl;
        std::cout << "Map commands" << std::endl;
        std::cout << "   FIND MAP-INSERT ERASE CLEAR SIZE TOMAP FROMMAP CREATEMAP" << std::endl;
        std::cout << "" << std::endl;
        std::cout << "Functional commands" << std::endl;
        std::cout << "   APPLY SELECT" << std::endl;
        std::cout << "" << std::endl;
        std::cout << "IO commands" << std::endl;
        std::cout << "   PRINT PROMPT PREAD PWRITE FWRITE " << std::endl;
        std::cout << "" << std::endl;
        std::cout << "String commands" << std::endl;
        std::cout << "   FORMAT CAT JOIN SUBSTR STRFIND STRCMP STRNCMP SPLIT SIZE CLEAR" << std::endl;
        std::cout << "" << std::endl;
        std::cout << "Type commands" << std::endl;
        std::cout << "   TOINT TOSTR TYPE " << std::endl;
        std::cout << "" << std::endl;
        std::cout << "Executioncommands" << std::endl;
        std::cout << "   EVAL CALL" << std::endl;
        std::cout << "" << std::endl;
        std::cout << "Environment" << std::endl;
        std::cout << "   NAMESPACES SETNS GETNS CD HELP SYSTEM" << std::endl;
        std::cout << "" << std::endl;
        std::cout << "Misc commands" << std::endl;
        std::cout << "   CLONE" << std::endl;
        std::cout << "Command: " << std::flush;
        std::string cmd;
        std::getline(std::cin, cmd);
        if (cmd == "q")
            return;
        auto it = machine.commands.find(cmd);
        if (it != machine.commands.end())
        {
            machine.help = true;
            std::cout << std::endl;
            (*it->second->funcptr)(machine);
            std::cout << std::endl;
            machine.help = false;
            std::getline(std::cin, cmd);
        }
        else
        {
            std::cout << "Help not found for " << cmd << std::endl;
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


