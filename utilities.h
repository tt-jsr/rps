#pragma once
#include <functional>
#include <cassert>

namespace std
{
    template<>
    struct hash<Object *>
    {
        size_t operator()(const Object* obj) const
        {
            if (obj->type == OBJECT_INTEGER)
            {
                int64_t v = ((const Integer *)obj)->value;
                std::hash<int64_t> h;
                return h(v);
            }
            if (obj->type == OBJECT_STRING)
            {
                std::string s = ((const String *)obj)->get();
                std::hash<std::string> h;
                return h(s);
            }
            assert(false);
        }
    };

    template<>
    struct less<Object *>
    {
        bool operator()(const Object* obj1, const Object* obj2) const
        {
            if (obj1->type == OBJECT_INTEGER && obj2->type == OBJECT_INTEGER)
            {
                return ((Integer *)obj1)->value < ((Integer *)obj2)->value;
            }
            if (obj1->type == OBJECT_STRING && obj2->type == OBJECT_STRING)
            {
                return ((String *)obj1)->get() < ((String *)obj2)->get();
            }
            return obj1 < obj2;
        }
    };

    template<>
    struct equal_to<Object *>
    {
        bool operator()(const Object* obj1, const Object* obj2) const
        {
            if (obj1->type == OBJECT_INTEGER && obj2->type == OBJECT_INTEGER)
            {
                return ((Integer *)obj1)->value == ((Integer *)obj2)->value;
            }
            if (obj1->type == OBJECT_STRING && obj2->type == OBJECT_STRING)
            {
                return ((String *)obj1)->get() == ((String *)obj2)->get();
            }
            return obj1 == obj2;
        }
    };
}

class Machine;
class Parser;

StringPtr MakeString();
IntegerPtr MakeInteger();
ProgramPtr MakeProgram();
NonePtr MakeNone();
ListPtr MakeList();
MapPtr MakeMap();
ObjectPtr Clone(ObjectPtr);

bool ToBool(Machine&, ObjectPtr);
void GetArgs(Machine&, std::vector<std::string>& args);

std::string ToStr(Machine&, ObjectPtr);
int64_t ToInt(Machine&, ObjectPtr);
std::string ToType(Machine&, ObjectPtr);


void split(const std::string& str, std::vector<std::string>& out, const std::string& delim, bool bCollapse = false);
void Import(Machine& machine, Parser& parser, const std::string& modname);
