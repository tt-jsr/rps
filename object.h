#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
#include "token.h"

namespace rps
{

class Machine;

class Object
{
public:
    Object(ObjectType t)
    :type(t)
    {}

    ~Object() {}

    ObjectType type;
    virtual bool IsToken(TokenType t) {return false;}
};

typedef std::shared_ptr<Object> ObjectPtr;

class Token : public Object
{
public:
    Token(TokenType t, const std::string& v)
    : Object(OBJECT_TOKEN)
    , value(v) 
    , tok_type(t)
    {
    }

    bool IsToken(TokenType t) override {return t == tok_type;}
    std::string value;
    TokenType tok_type;
};

typedef std::shared_ptr<Token> TokenPtr;

class String : public Object
{
public:
    String(const std::string&s) 
    : Object(OBJECT_STRING)
    , value(s)
    {}
    void set(const std::string&);
    const std::string& get() const { return value;}

    std::string value;
};

typedef std::shared_ptr<String> StringPtr;

class Integer : public Object
{
public:
    Integer(int64_t n) : Object(OBJECT_INTEGER), value(n) {}
    int64_t value;
};

typedef std::shared_ptr<Integer> IntegerPtr;

class None : public Object
{
public:
    None() : Object(OBJECT_NONE) {}
};

typedef std::shared_ptr<None> NonePtr;

class List : public Object
{
public:
    List() : Object(OBJECT_LIST) {}
    std::vector<ObjectPtr> items;
};

typedef std::shared_ptr<List> ListPtr;

class Map : public Object
{
public:
    Map() : Object(OBJECT_MAP) {}
    std::unordered_map<ObjectPtr, ObjectPtr> items;
};

typedef std::shared_ptr<Map> MapPtr;

class Program;
typedef std::shared_ptr<Program> ProgramPtr;

class Command : public Object
{
public:
    Command(const std::string& cmd, void (*f)(Machine&)) 
    : Object(OBJECT_COMMAND)
    , value(cmd)
    , funcptr(f)
    {}
    std::string value;
    void (*funcptr)(Machine&);

    ProgramPtr program;
};

typedef std::shared_ptr<Command> CommandPtr;

class Program : public Object
{
public:
    Program() : Object(OBJECT_PROGRAM) {}
    std::vector<ObjectPtr> program;
    std::string module_name;
    std::unordered_map<std::string, ObjectPtr> *pLocals;
    ProgramPtr enclosingProgram;
};


class If : public Object
{
public:
    If() : Object(OBJECT_IF) {}
    std::vector<ObjectPtr> cond;
    std::vector<ObjectPtr> then;
    std::vector<ObjectPtr> els;
};

typedef std::shared_ptr<If> IfPtr;

class For : public Object
{
public:
    For() : Object(OBJECT_FOR) {}
    std::vector<ObjectPtr> program;
};

typedef std::shared_ptr<For> ForPtr;

class While : public Object
{
public:
    While() : Object(OBJECT_WHILE) {}
    std::vector<ObjectPtr> program;
    std::vector<ObjectPtr> cond;
};

typedef std::shared_ptr<While> WhilePtr;

} // namespace rps

