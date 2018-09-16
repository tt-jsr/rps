#include <vector>
#include <unordered_map>
#include <exception>
#include <iostream>
#include <sstream>
#include <cassert>
#include <memory>
#include <algorithm>
#include "token.h"
#include "object.h"
#include "module.h"
#include "machine.h"
#include "commands.h"
#include "utilities.h"

// [list] "str"    => [list]
// [list] int      => [list]
// [list] <<prog>> => [list]
// [list] {map}    => [list]
// [list] [list]   => [list]
void APPEND(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "APPEND: Append the object at L0 to the list on L1" << std::endl;
        std::cout << "[list] \"obj\" APPEND => [list]" << std::endl;
        return;
    }

    ListPtr lp;
    ObjectPtr optr;
    stack_required(machine, "APPEND", 2);
    throw_required(machine, "APPEND", 1, OBJECT_LIST);
    machine.pop(optr);
    machine.pop(lp);
    lp->items.push_back(optr);
    machine.push(lp);
}

// [list] int(idx) => obj
void GET(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "GET: Get the object object in a list at the given index" << std::endl;
        std::cout << "[list] idx GET => obj" << std::endl;
        return;
    }

    stack_required(machine, "GET", 2);
    throw_required(machine, "GET", 0, OBJECT_INTEGER);
    throw_required(machine, "GET", 1, OBJECT_LIST);

    ListPtr lp;
    int64_t idx;

    machine.pop(idx);
    machine.pop(lp);
    if (idx < 0)
    {
        idx = lp->items.size() + idx;
    }
    if (idx < 0 || idx >= lp->items.size())
    {
        machine.push(lp);
        throw std::runtime_error("GET: Index out of range for List");
    }
    ObjectPtr p = lp->items[idx];
    machine.push(p);
    return;
}

// {map} "str" => obj
// {map} int   => obj
void FIND(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "FIND: Find an item in a map" << std::endl;
        std::cout << "{map} key onError FIND => obj" << std::endl;
        std::cout << "key: Either a string or integer key" << std::endl;
        std::cout << "onError: If the key is not found, this is the value placed on L0" << std::endl;
        return;
    }

    stack_required(machine, "FIND", 3);
    throw_required(machine, "FIND", 2, OBJECT_MAP);
    if (machine.peek(1)->type != OBJECT_INTEGER && machine.peek(1)->type != OBJECT_STRING)
        throw std::runtime_error("GET: requires Integer or String key for Map");
    ObjectPtr key;
    MapPtr mp;
    ObjectPtr onError;

    machine.pop(onError);
    machine.pop(key);
    machine.pop(mp);
    auto it = mp->items.find(key);
    if (it == mp->items.end())
        EVAL(machine, onError);
    else
        machine.push(it->second);
}

// [list] idx value => [list]
// idx: int
// value: int | "str" | [list] | {map} | <<prog>>
void LIST_INSERT(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "LIST-INSERT: Insert an item into a list" << std::endl;
        std::cout << "[list] idx obj => [list]" << std::endl;
        return;
    }

    stack_required(machine, "LIST-INSERT", 3);
    throw_required(machine, "LIST-INSERT", 2, OBJECT_LIST);
    throw_required(machine, "LIST-INSERT", 1, OBJECT_INTEGER);

    ListPtr lp;
    int64_t idx;
    ObjectPtr element;

    machine.pop(element);
    machine.pop(idx);
    machine.pop(lp);
    if (idx < 0)
    {
        idx = lp->items.size() + idx;
    }
    if (idx >= lp->items.size())
    {
        machine.push(lp);
        machine.push(idx);
        machine.push(element);
        throw std::runtime_error("List insert: Index out of range");
    }
    lp->items.insert(lp->items.begin()+idx, element);
    machine.push(lp);
}

// {map} [list] => {map}
//
// list is a tuple of: [key, value]
// key: int | "str"
// value: int | "str" | {map} | [list] | <<prog>>
void MAP_INSERT(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "MAP-INSERT: Insert into a map" << std::endl;
        std::cout << "{map} [k,v] => {map}" << std::endl;
        std::cout << "list: A tuple of a Key-Value pair" << std::endl;
        return;
    }

    stack_required(machine, "MAP-INSERT",  2);
    throw_required(machine, "MAP-INSERT",  1, OBJECT_MAP);
    throw_required(machine, "MAP-INSERT",  0, OBJECT_LIST);

    MapPtr mp;
    ListPtr kv;
    machine.pop(kv);
    machine.pop(mp);
    mp->items[kv->items[0]] =  kv->items[1];
    machine.push(mp);
}

void INSERT(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "Help not available" << std::endl;
        return;
    }

    if (machine.stack_.size() >= 3 
            && machine.peek(2)->type == OBJECT_LIST 
            && machine.peek(1)->type == OBJECT_INTEGER)
    {
        LIST_INSERT(machine);
        return;
    }
    if (machine.stack_.size() >= 2 
            && machine.peek(1)->type == OBJECT_MAP 
            && machine.peek(0)->type == OBJECT_LIST)
    {
        MAP_INSERT(machine);
        return;
    }
    std::runtime_error("INSERT: cannot detect map or list insert");
}

// [list] idx => [list]
//    idx: int
// {map} key => {map}
//    key: int | "str"
void ERASE(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "ERASE: Erase an item from a list or a map" << std::endl;
        std::cout << "[list] idx ERASE => [list]" << std::endl;
        std::cout << "{map} key ERASE => {map}" << std::endl;
        return;
    }

    stack_required(machine, "ERASE",  2);
    if (machine.peek(1)->type == OBJECT_LIST)
    {
        if (machine.peek(0)->type != OBJECT_INTEGER)
            throw std::runtime_error("ERASE: requires Integer argument for List");

        ListPtr lp;
        int64_t idx;
        machine.pop(idx);
        machine.pop(lp);
        if (idx < 0)
        {
            idx = lp->items.size() + idx;
        }
        if (idx < 0 || idx >= lp->items.size())
        {
            machine.push(lp);
            machine.push(idx);
            throw std::runtime_error("ERASE: Index out of range for List");
        }
        lp->items.erase(lp->items.begin() + idx);
        machine.push(lp);
        return;
    }
    if (machine.peek(1)->type == OBJECT_MAP)
    {
        if (machine.peek(0)->type != OBJECT_INTEGER && machine.peek(0)->type != OBJECT_STRING)
            throw std::runtime_error("ERASE: requires String or Integer argument for Map");

        MapPtr mp;
        ObjectPtr key;
        machine.pop(key);
        machine.pop(mp);
        mp->items.erase(key);
        machine.push(mp);
        return;
    }
    throw std::runtime_error("ERASE: requires List  or Map argument");
}

// [list] => [list]
// {map}  => {map}
void CLEAR(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "CLEAR: Clear a list or map" << std::endl;
        std::cout << "[list] CLEAR => []" << std::endl;
        std::cout << "{map} CLEAR => {}" << std::endl;
        return;
    }

    if (machine.stack_.size() < 1)
        throw std::runtime_error("CLEAR requires a List argument");
    if (machine.peek(0)->type == OBJECT_LIST)
    {
        ListPtr lp;
        machine.pop(lp);
        lp->items.clear();
        machine.push(lp);
        return;
    }
    if (machine.peek(0)->type == OBJECT_MAP)
    {
        MapPtr mp;
        machine.pop(mp);
        mp->items.clear();
        machine.push(mp);
        return;
    }
    throw std::runtime_error("CLEAR: requires List or Map argument");
}

// [list]    => int
// {map}     => int
// "str"     => int
// <<prog>>  => int
void SIZE(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "SIZE: Return the size of an object" << std::endl;
        std::cout << "[list] SIZE => int" << std::endl;
        std::cout << "{map} SIZE => int" << std::endl;
        std::cout << "\"str\" SIZE => int" << std::endl;
        std::cout << "<<prog>> SIZE => int" << std::endl;
        return;
    }

    if (machine.stack_.size() < 1)
        throw std::runtime_error("SIZE requires a List argument");
    if (machine.peek(0)->type == OBJECT_LIST)
    {
        ListPtr lp;
        machine.pop(lp);
        int64_t sz = lp->items.size();
        machine.push(sz);
        return;
    }
    if (machine.peek(0)->type == OBJECT_MAP)
    {
        MapPtr mp;
        machine.pop(mp);
        int64_t sz = mp->items.size();
        machine.push(sz);
        return;
    }
    if (machine.peek(0)->type == OBJECT_STRING)
    {
        ObjectPtr optr;
        machine.pop(optr);
        int64_t sz = ((String *)optr.get())->value.size();
        machine.push(sz);
        return;
    }
    if (machine.peek(0)->type == OBJECT_PROGRAM)
    {
        ObjectPtr optr;
        machine.pop(optr);
        int64_t sz = ((Program *)optr.get())->program.size();
        machine.push(sz);
        return;
    }
    throw std::runtime_error("SIZE: requires [list], {map}, \"str\", or <<prog>> argument");
}

// [list] => obj
void FIRST(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "FIRST: Get the fist element of a list" << std::endl;
        std::cout << "[list] FIRST => obj" << std::endl;
        return;
    }

    stack_required(machine, "FIRST", 1);
    throw_required(machine, "FIRST", 0,  OBJECT_LIST);
    machine.push(0);
    GET(machine);
}

// [list] => obj
void SECOND(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "SECOND: Get the second element of a list" << std::endl;
        std::cout << "[list] SECOND => obj" << std::endl;
        return;
    }

    stack_required(machine, "SECOND", 1);
    throw_required(machine, "SECOND", 0, OBJECT_LIST);
    machine.push(1);
    GET(machine);
}

// obj, obj...  int => [list]
void TOLIST(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "TOLIST: Take n items from the stack and create a list" << std::endl;
        std::cout << "obj1 obj2 obj3... nitems TOLIST => [list]" << std::endl;
        return;
    }

    ListPtr lp;
    lp = MakeList();
    int64_t num;
    machine.pop(num);
    while (num--)
    {
        ObjectPtr obj;
        machine.pop(obj);
        lp->items.push_back(obj);
    }
    std::reverse(lp->items.begin(), lp->items.end());
    machine.push(lp);
}

// [list], [list], [list]... int => {map}
void TOMAP(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "TOMAP: Take n tuples from the stack and create a map" << std::endl;
        std::cout << "[k,v] [k,v] [k,v]... nitems TOMAP => {map}" << std::endl;
        return;
    }

    MapPtr mp;
    mp = MakeMap();
    int64_t num;
    machine.pop(num);
    while (num--)
    {
        ListPtr lp;
        machine.pop(lp);
        mp->items[lp->items[0]] = lp->items[1];
    }
    machine.push(mp);
}

// [list] => obj, obj. obj,...
void FROMLIST(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "FROMLIST: Push all items of a list to the stack" << std::endl;
        std::cout << "[list] FROMLIST => obj1, obj2, obj3..." << std::endl;
        return;
    }

    ListPtr lp;
    machine.pop(lp);
    for (ObjectPtr op : lp->items)
    {
        machine.push(op);
    }
}

// {map} => [list], [list], [list], ...
void FROMMAP(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "FROMMAP: Push all key/values of a map onto the stack" << std::endl;
        std::cout << "{map} FROMMAP => [k, v] [k, v] [k,v]..." << std::endl;
        return;
    }

    MapPtr mp;
    machine.pop(mp);
    for (auto& pr :mp->items)
    {
        ListPtr lp = MakeList();
        lp->items.push_back(pr.first);
        lp->items.push_back(pr.second);
        machine.push(lp);
    }
}

// => []
void CREATELIST(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "CREATELIST: Create a list" << std::endl;
        std::cout << "CREATELIST => []" << std::endl;
        return;
    }

    ListPtr lp = MakeList();
    machine.push(lp);
}

// => {}
void CREATEMAP(Machine& machine)
{
    if (machine.help)
    {
        std::cout << "CREATEMAP: Create a map" << std::endl;
        std::cout << "CREATEMAP => {}" << std::endl;
        return;
    }

    MapPtr mp = MakeMap();
    machine.push(mp);
}

