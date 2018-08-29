#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
#include "token.h"

class Machine;

class Object
{
public:
    Object(ObjectType t, Token tok)
    :type(t)
    ,token(tok)
    {}

    ~Object() {}

    ObjectType type;
    Token token;
};

typedef std::shared_ptr<Object> ObjectPtr;

class String : public Object
{
public:
    String(const std::string&s) : Object(OBJECT_STRING, TOKEN_DATA), value(s) {}
    std::string value;
};

typedef std::shared_ptr<String> StringPtr;

class Integer : public Object
{
public:
    Integer(int64_t n) : Object(OBJECT_INTEGER, TOKEN_DATA), value(n) {}
    int64_t value;
};

typedef std::shared_ptr<Integer> IntegerPtr;

class List : public Object
{
public:
    List() : Object(OBJECT_LIST, TOKEN_DATA) {}
    std::vector<ObjectPtr> items;
};

typedef std::shared_ptr<List> ListPtr;

class Command : public Object
{
public:
    Command(const std::string& cmd, void (*f)(Machine&)) 
    : Object(OBJECT_COMMAND, TOKEN_COMMAND)
    , value(cmd)
    , funcptr(f)
    {}
    std::string value;
    void (*funcptr)(Machine&);
};

typedef std::shared_ptr<Command> CommandPtr;

class Program : public Object
{
public:
    Program() : Object(OBJECT_PROGRAM, TOKEN_DATA) {}
    std::vector<ObjectPtr> program;
    std::unordered_map<std::string, ObjectPtr> locals;
    std::string module_name;
};

typedef std::shared_ptr<Program> ProgramPtr;


