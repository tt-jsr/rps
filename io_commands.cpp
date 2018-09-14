#include <vector>
#include <unordered_map>
#include <exception>
#include <iostream>
#include <sstream>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <limits>
#include "token.h"
#include "object.h"
#include "module.h"
#include "machine.h"
#include "commands.h"
#include "utilities.h"

// "str" =>
void PRINT(Machine& machine)
{
    if (machine.stack_.size() < 1)
        throw std::runtime_error("PRINT: Requires object at L0");

    ObjectPtr optr;
    machine.pop(optr);

    std::cout << ToStr(machine, optr) << std::endl;
}

// "str" => "str"
void PROMPT(Machine& machine)
{
    if (machine.stack_.size() < 1)
        throw std::runtime_error("PROMPT: Requires string at L0");

    throw_required(machine, "PROMPT", 0, OBJECT_STRING);

    std::string prompt;
    machine.pop(prompt);

    std::cout << prompt << std::flush;

    std::string inbuf;
    std::getline(std::cin, inbuf);
    machine.push(inbuf);
}

// cmd => [list]
// cmd opt1.. => PREAD
//   opt: "--limit=n"
void PREAD(Machine& machine)
{
   stack_required(machine, "PREAD", 1);
   throw_required(machine, "PREAD", 0, OBJECT_STRING);

   std::string opt;
   std::string cmd;
   machine.pop(opt);
   int limit = std::numeric_limits<int>::max();
   while (strncmp(opt.c_str(), "--", 2) == 0)
   {
       if (strncmp(opt.c_str(), "--limit=", 8) == 0)
       {
            limit = std::stoi(&opt.c_str()[8]);
       }
       throw_required(machine, "PREAD", 0, OBJECT_STRING);
       machine.pop(opt);
   }
   cmd = opt;
   ListPtr ret = MakeList();

   FILE *fp = popen(cmd.c_str(), "r");
   if (fp)
   {
       char buf[10240];
       for (int count = 0; count < limit && !feof(fp); ++count)
       {
           if (fgets(buf, sizeof(buf), fp))
           {
               size_t l = strlen(buf);
               if (buf[l-1] == '\n')
                   buf[l-1] = '\0';
               StringPtr sp = MakeString();
               sp->value = buf;
               ret->items.push_back(sp);
           }
       }
       pclose(fp);
   }
   machine.push(ret);

}

void SYSTEM(Machine& machine)
{
    stack_required(machine, "SYSTEM", 1);
    throw_required(machine, "SYSTEM", 0, OBJECT_STRING);
    std::string cmd;
    machine.pop(cmd);
    system(cmd.c_str());
}
