#pragma once
#include <functional>
#include <cassert>

namespace std
{
    template<>
    struct hash<rps::Object *>
    {
        size_t operator()(const rps::Object* obj) const
        {
            if (obj->type == rps::OBJECT_INTEGER)
            {
                int64_t v = ((const rps::Integer *)obj)->value;
                std::hash<int64_t> h;
                return h(v);
            }
            if (obj->type == rps::OBJECT_STRING)
            {
                std::string s = ((const rps::String *)obj)->get();
                std::hash<std::string> h;
                return h(s);
            }
            assert(false);
        }
    };

    template<>
    struct less<rps::Object *>
    {
        bool operator()(const rps::Object* obj1, const rps::Object* obj2) const
        {
            if (obj1->type == rps::OBJECT_INTEGER && obj2->type == rps::OBJECT_INTEGER)
            {
                return ((rps::Integer *)obj1)->value < ((rps::Integer *)obj2)->value;
            }
            if (obj1->type == rps::OBJECT_STRING && obj2->type == rps::OBJECT_STRING)
            {
                return ((rps::String *)obj1)->get() < ((rps::String *)obj2)->get();
            }
            return obj1 < obj2;
        }
    };

    template<>
    struct equal_to<rps::Object *>
    {
        bool operator()(const rps::Object* obj1, const rps::Object* obj2) const
        {
            if (obj1->type == rps::OBJECT_INTEGER && obj2->type == rps::OBJECT_INTEGER)
            {
                return ((rps::Integer *)obj1)->value == ((rps::Integer *)obj2)->value;
            }
            if (obj1->type == rps::OBJECT_STRING && obj2->type == rps::OBJECT_STRING)
            {
                return ((rps::String *)obj1)->get() == ((rps::String *)obj2)->get();
            }
            return obj1 == obj2;
        }
    };
}

namespace rps
{

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

} // namespace rps

