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
    ListPtr lp;
    ObjectPtr optr;
    if (machine.stack_.size() < 2)
        throw std::runtime_error("APPEND requires two arguments");
    if (machine.peek(1)->type != OBJECT_LIST)
        throw std::runtime_error("APPEND: requires List argument");
    machine.pop(optr);
    machine.pop(lp);
    lp->items.push_back(optr);
    machine.push(lp);
}

// [list] int(idx) => obj
void GET(Machine& machine)
{
    if (machine.stack_.size() < 2)
        throw std::runtime_error("GET requires two arguments");
    if (machine.peek(1)->type == OBJECT_LIST)
    {
        if (machine.peek(0)->type != OBJECT_INTEGER)
            throw std::runtime_error("GET: requires Integer argument for List");
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
            throw std::runtime_error("GET: Index out of range for List");
        }
        ObjectPtr p = lp->items[idx];
        machine.push(p);
        return;
    }
    throw std::runtime_error("GET: requires List argument");
}

// {map} "str" => obj
// {map} int   => obj
void FIND(Machine& machine)
{
    if (machine.stack_.size() < 3)
        throw std::runtime_error("FIND requires three arguments");
    if (machine.peek(2)->type == OBJECT_MAP)
    {
        if (machine.peek(1)->type != OBJECT_INTEGER && machine.peek(0)->type != OBJECT_STRING)
            throw std::runtime_error("GET: requires Integer or String argument for Map");
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
        return;
    }
    throw std::runtime_error("FIND: requires Map argument");
}

// [list] idx value => [list]
// idx: int
// value: int | "str" | [list] | {map} | <<prog>>
void LIST_INSERT(Machine& machine)
{
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
    if (machine.stack_.size() < 1)
        throw std::runtime_error("FIRST requires list argument");
    if (machine.peek(0)->type != OBJECT_LIST)
        throw std::runtime_error("FIRST: requires List argument");
    machine.push(0);
    GET(machine);
}

// [list] => obj
void SECOND(Machine& machine)
{
    if (machine.stack_.size() < 1)
        throw std::runtime_error("SECOND requires list argument");
    if (machine.peek(0)->type != OBJECT_LIST)
        throw std::runtime_error("SECOND: requires List argument");
    machine.push(1);
    GET(machine);
}

// obj, obj...  int => [list]
void TOLIST(Machine& machine)
{
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
    ListPtr lp = MakeList();
    machine.push(lp);
}

// => {}
void CREATEMAP(Machine& machine)
{
    MapPtr mp = MakeMap();
    machine.push(mp);
}

