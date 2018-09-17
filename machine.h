#pragma once
#include <unordered_map>
#include <vector>
#include <sstream>

class Command;
typedef std::shared_ptr<Command> CommandPtr;

class Machine
{
public:
    Machine();
    void push(ObjectPtr& ptr);
    std::unordered_map<std::string, Module> modules_;
    std::vector<ObjectPtr> stack_;
    std::string current_module_;
    ProgramPtr current_program;

    void CreateModule(const std::string& name);
    std::ostream& helpstrm();

    ObjectPtr& peek();
    ObjectPtr& peek(size_t n);
    std::string toStr(ObjectPtr& optr);
    void pop(int64_t&);
    void push(int64_t);
    void pop(std::string&);
    void push(const std::string&);
    void pop(ObjectPtr&);
    void pop(ListPtr&);
    void push(ListPtr&);
    void pop(MapPtr&);
    void push(MapPtr&);
    void pop();

    // settings
    size_t maxwidth;
    bool debug;
    bool help;
    std::unordered_map<std::string, CommandPtr> commands;
    std::unordered_map<std::string, std::vector<std::string>> categories;
    std::stringstream hstrm;
};


void stack_required(Machine& machine, const char *f, int depth);
void throw_required(Machine& machine, const char *f, int level, ObjectType t);
void Execute(Machine&);
void Execute(Machine&, ObjectPtr);

