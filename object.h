#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
#include "token.h"

class Machine;

class Object
{
public:
    Object(ObjectType t, TokenType tok)
    :type(t)
    ,token(tok)
    ,bSuppressInteractivePrint(false)
    {}

    ~Object() {}

    ObjectType type;
    TokenType token;
    bool bSuppressInteractivePrint;
};

typedef std::shared_ptr<Object> ObjectPtr;

bool operator==(ObjectPtr, ObjectPtr);

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

class Map : public Object
{
public:
    Map() : Object(OBJECT_MAP, TOKEN_DATA) {}
    std::unordered_map<ObjectPtr, ObjectPtr> items;
};

typedef std::shared_ptr<Map> MapPtr;

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

class Program;
typedef std::shared_ptr<Program> ProgramPtr;

class Program : public Object
{
public:
    Program() : Object(OBJECT_PROGRAM, TOKEN_DATA) {}
    std::vector<ObjectPtr> program;
    std::string module_name;
    std::unordered_map<std::string, ObjectPtr> *pLocals;
    ProgramPtr enclosingProgram;
};


class If : public Object
{
public:
    If() : Object(OBJECT_IF, TOKEN_STATEMENT) {}
    std::vector<ObjectPtr> cond;
    std::vector<ObjectPtr> then;
    std::vector<ObjectPtr> els;
};

typedef std::shared_ptr<If> IfPtr;

class For : public Object
{
public:
    For() : Object(OBJECT_FOR, TOKEN_STATEMENT) {}
    std::vector<ObjectPtr> program;
};

typedef std::shared_ptr<For> ForPtr;

class While : public Object
{
public:
    While() : Object(OBJECT_WHILE, TOKEN_STATEMENT) {}
    std::vector<ObjectPtr> program;
    std::vector<ObjectPtr> cond;
};

typedef std::shared_ptr<While> WhilePtr;
