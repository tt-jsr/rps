#include <vector>
#include <unordered_map>
#include <exception>
#include <iostream>
#include <sstream>
#include <cassert>
#include "token.h"
#include "object.h"
#include "module.h"
#include "machine.h"
#include "commands.h"
#include "utilities.h"

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
            if (pCommand->program)
                EVAL(machine, pCommand->program);
            else
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
        machine.helpstrm() << "EVAL: Evaluate an object";
        machine.helpstrm() << "obj EVAL => ...";
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
        machine.helpstrm() << "CALL: Call a program";
        machine.helpstrm() << "\"name\" CALL => ...";
        machine.helpstrm() << "Equivilent to name RCL EVAL";
        return;
    }
    RCL(machine);
    EVAL(machine);
}


