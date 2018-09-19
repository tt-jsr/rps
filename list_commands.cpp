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

void APPEND(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "APPEND: Append the object at L0 to the list on L1" << std::endl;
        machine.helpstrm() << "[list] \"obj\" APPEND => [list]" << std::endl;
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

void GET(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "GET: Get the object in a list at the given index" << std::endl;
        machine.helpstrm() << "[list] idx GET => obj" << std::endl;
        machine.helpstrm() << "If idx < 0 it is taken from the end of the list" << std::endl;
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

void SUBLIST(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "SUBLIST: Push a sublist";
        machine.helpstrm() << "[list] startpos length SUBLIST => [list]";
        machine.helpstrm() << "If startpos is < 0, it is taken from the end of the list";
        machine.helpstrm() << "If length < 0 or longer then available, copy until the end of the list";
        return;
    }

    stack_required(machine, "SUBLIST", 3);
    throw_required(machine, "SUBLIST", 0, OBJECT_INTEGER);
    throw_required(machine, "SUBLIST", 1, OBJECT_INTEGER);
    throw_required(machine, "SUBLIST", 2, OBJECT_LIST);

    int64_t startpos, length;
    ListPtr lp;
    machine.pop(length);
    machine.pop(startpos);
    machine.pop(lp);
    if (startpos < 0)
    {
        startpos = lp->items.size() + startpos;
    }
    if (startpos < 0 || startpos >= lp->items.size())
    {
        machine.push(lp);
        throw std::runtime_error("SUBLIST: startpos out of range for List");
    }
    if ((startpos+length) >= lp->items.size())
        length = -1;
    ListPtr result = MakeList();
    if (length < 0)
        std::copy(lp->items.begin()+startpos, lp->items.end(), std::back_inserter(result->items));
    else
        std::copy(lp->items.begin()+startpos, lp->items.begin()+startpos+length, std::back_inserter(result->items));
    machine.push(result);
}
void LINSERT(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "LINSERT: Insert an item into a list";
        machine.helpstrm() << "[list] idx obj => [list]";
        return;
    }

    stack_required(machine, "LINSERT", 3);
    throw_required(machine, "LINSERT", 2, OBJECT_LIST);
    throw_required(machine, "LINSERT", 1, OBJECT_INTEGER);

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
void INSERT(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "Help not available";
        return;
    }

    if (machine.stack_.size() >= 3 
            && machine.peek(2)->type == OBJECT_LIST 
            && machine.peek(1)->type == OBJECT_INTEGER)
    {
        LINSERT(machine);
        return;
    }
    if (machine.stack_.size() >= 2 
            && machine.peek(1)->type == OBJECT_MAP 
            && machine.peek(0)->type == OBJECT_LIST)
    {
        MINSERT(machine);
        return;
    }
    std::runtime_error("INSERT: cannot detect map or list insert");
}

void ERASE(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "ERASE: Erase an item from a list or a map";
        machine.helpstrm() << "[list] idx ERASE => [list]";
        machine.helpstrm() << "{map} key ERASE => {map}";
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

void CLEAR(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "CLEAR: Clear a list or map";
        machine.helpstrm() << "[list] CLEAR => []";
        machine.helpstrm() << "{map} CLEAR => {}";
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

void SIZE(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "SIZE: Return the size of an object";
        machine.helpstrm() << "[list] SIZE => int";
        machine.helpstrm() << "{map} SIZE => int";
        machine.helpstrm() << "\"str\" SIZE => int";
        machine.helpstrm() << "<<prog>> SIZE => int";
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

void FIRST(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "FIRST: Get the fist element of a list";
        machine.helpstrm() << "[list] FIRST => obj";
        return;
    }

    stack_required(machine, "FIRST", 1);
    throw_required(machine, "FIRST", 0,  OBJECT_LIST);
    machine.push(0);
    GET(machine);
}

void SECOND(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "SECOND: Get the second element of a list";
        machine.helpstrm() << "[list] SECOND => obj";
        return;
    }

    stack_required(machine, "SECOND", 1);
    throw_required(machine, "SECOND", 0, OBJECT_LIST);
    machine.push(1);
    GET(machine);
}

void HEAD(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "HEAD: Get nitems from the head of the list";
        machine.helpstrm() << "[list] n HEAD => [list]";
        return;
    }

    stack_required(machine, "HEAD", 2);
    throw_required(machine, "HEAD", 0, OBJECT_INTEGER);
    throw_required(machine, "HEAD", 1, OBJECT_LIST);

    ListPtr lp;
    int64_t n;
    machine.pop(n);
    machine.pop(lp);
    if (n >=lp->items.size())
        n = lp->items.size()-1;
    ListPtr result = MakeList();
    std::copy(lp->items.begin(), lp->items.begin()+n, std::back_inserter(result->items));
    machine.push(result);
}

void TAIL(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "TAIL: Get nitems from the tail of the list";
        machine.helpstrm() << "[list] n TAIL => [list]";
        return;
    }

    stack_required(machine, "TAIL", 2);
    throw_required(machine, "TAIL", 0, OBJECT_INTEGER);
    throw_required(machine, "TAIL", 1, OBJECT_LIST);

    ListPtr lp;
    int64_t n;
    machine.pop(n);
    machine.pop(lp);
    n = lp->items.size() - n;
    if (n < 0)
        n = 0;

    ListPtr result = MakeList();
    std::copy(lp->items.begin()+n, lp->items.end(), std::back_inserter(result->items));
    machine.push(result);
}

void TOLIST(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "TOLIST: Take n items from the stack and create a list";
        machine.helpstrm() << "obj1 obj2 obj3... nitems TOLIST => [list]";
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

void FROMLIST(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "FROMLIST: Push all items of a list to the stack";
        machine.helpstrm() << "[list] FROMLIST => obj1, obj2, obj3...";
        return;
    }

    ListPtr lp;
    machine.pop(lp);
    for (ObjectPtr op : lp->items)
    {
        machine.push(op);
    }
}

void CREATELIST(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "CREATELIST: Create a list";
        machine.helpstrm() << "CREATELIST => []";
        return;
    }

    ListPtr lp = MakeList();
    machine.push(lp);
}

void ZIP(Machine& machine)
{
    throw std::runtime_error("ZIP not yet implemented");
}

void UNZIP(Machine& machine)
{
    throw std::runtime_error("ZIP not yet implemented");
}

//*******************************************************************
// maps

void CREATEMAP(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "CREATEMAP: Create a map";
        machine.helpstrm() << "CREATEMAP => {}";
        return;
    }

    MapPtr mp = MakeMap();
    machine.push(mp);
}

void KEYS(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "KEYS: Get a list of keys";
        machine.helpstrm() << "{map} KEYS => []";
        return;
    }
    stack_required(machine, "KEYS", 1);
    throw_required(machine, "KEYS", 0, OBJECT_MAP);

    ObjectPtr optr;
    machine.pop(optr);
    Map *pMap = (Map *)optr.get();
    ListPtr lp = MakeList();
    for (auto& pr : pMap->items)
    {
        lp->items.push_back(pr.first);
    }
    machine.push(lp);
}

void VALUES(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "VALUES: Get a list of values";
        machine.helpstrm() << "{map} VALUES => []";
        return;
    }
    stack_required(machine, "VALUES", 1);
    throw_required(machine, "VALUES", 0, OBJECT_MAP);

    ObjectPtr optr;
    machine.pop(optr);
    Map *pMap = (Map *)optr.get();
    ListPtr lp = MakeList();
    for (auto& pr : pMap->items)
    {
        lp->items.push_back(pr.second);
    }
    machine.push(lp);
}

void FIND(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "FIND: Find an item in a map";
        machine.helpstrm() << "{map} key onError FIND => obj";
        machine.helpstrm() << "key: Either a string or integer key";
        machine.helpstrm() << "onError: If the key is not found, this is the value placed on L0";
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


void MINSERT(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "MINSERT: Insert into a map";
        machine.helpstrm() << "{map} [k,v] => {map}";
        machine.helpstrm() << "list: A tuple of a Key-Value pair";
        return;
    }

    stack_required(machine, "MINSERT",  2);
    throw_required(machine, "MINSERT",  1, OBJECT_MAP);
    throw_required(machine, "MINSERT",  0, OBJECT_LIST);

    MapPtr mp;
    ListPtr kv;
    machine.pop(kv);
    machine.pop(mp);
    mp->items[kv->items[0]] =  kv->items[1];
    machine.push(mp);
}

void TOMAP(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "TOMAP: Take n tuples from the stack and create a map";
        machine.helpstrm() << "[k,v] [k,v] [k,v]... nitems TOMAP => {map}";
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
void FROMMAP(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "FROMMAP: Push all key/values of a map onto the stack";
        machine.helpstrm() << "{map} FROMMAP => [k, v] [k, v] [k,v]...";
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

