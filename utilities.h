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
                std::string s = ((const String *)obj)->value;
                std::hash<std::string> h;
                return h(s);
            }
            assert(false);
        }
    };
}

class Machine;

StringPtr MakeString();
IntegerPtr MakeInteger();
ProgramPtr MakeProgram();
ListPtr MakeList();
MapPtr MakeMap();
ObjectPtr Clone(ObjectPtr);

bool ToBool(Machine&, ObjectPtr);

std::string ToStr(Machine&, ObjectPtr);
int64_t ToInt(Machine&, ObjectPtr);
std::string ToType(Machine&, ObjectPtr);
