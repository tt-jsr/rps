#pragma once
#include <unordered_map>
#include <vector>

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

    // settings
    size_t list_maxcount;
    size_t map_maxcount;
    bool debug;
};


void throw_required(Machine& machine, const char *f, int level, ObjectType t);
void Execute(Machine&);
void Execute(Machine&, ObjectPtr);

