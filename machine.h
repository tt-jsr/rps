#pragma once
#include <unordered_map>
#include <vector>
#include <sstream>

namespace rps
{

extern bool bInterrupt;

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
    void pop(ProgramPtr&);
    void push(ProgramPtr&);
    void pop();

    // settings
    int64_t GetProperty(const std::string& name, int64_t def);
    std::string GetProperty(const std::string& name, const std::string& def);
    void SetProperty(const std::string& name, int64_t value);
    void SetProperty(const std::string& name, const std::string& value);
    //size_t viewwidth;
    //bool debug;
    //bool help;
    //bool shellExit;
    std::unordered_map<std::string, CommandPtr> commands;
    std::unordered_map<std::string, std::vector<std::string>> categories;
    std::unordered_map<std::string, ObjectPtr> properties;
    std::stringstream hstrm;
};


void stack_required(Machine& machine, const char *f, int depth);
void throw_required(Machine& machine, const char *f, int level, ObjectType t);
void Execute(Machine&);
void Execute(Machine&, ObjectPtr);

class Program;
typedef std::shared_ptr<Program> ProgramPtr;

void Category(Machine& machine, const std::string& cat, const std::string& name);
void AddCommand(Machine& machine, const std::string& name, void (*funcptr)(Machine&));
void AddCommand(Machine& machine, const std::string&, ProgramPtr);
void RemoveCommand(Machine& machine, const std::string&);
void ShowHelp(Machine& machine, CommandPtr cmd);

} // namespace rps

