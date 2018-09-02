#include "object.h"


bool operator==(ObjectPtr obj1, ObjectPtr obj2)
{
    if (obj1->type == OBJECT_INTEGER && obj2->type == OBJECT_INTEGER)
    {
        return ((Integer *)obj1.get())->value == ((Integer *)obj2.get())->value;
    }
    if (obj1->type == OBJECT_STRING && obj2->type == OBJECT_STRING)
    {
        return ((String *)obj1.get())->value == ((String *)obj2.get())->value;
    }
    return false;
}
