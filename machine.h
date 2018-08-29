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
    void CreateModule(const std::string& name);

    ObjectPtr& peek();
    ObjectPtr& peek(size_t n);
    std::string toStr(ObjectPtr& optr);
    void pop(int64_t&);
    void push(int64_t);
    void pop(std::string&);
    void push(const std::string&);
    void pop(ObjectPtr&);
};

void EVAL(Machine&);
void CALL(Machine&);

