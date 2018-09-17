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

void PRINT(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "PRINT: Print the object at L0 according to the rules of TOSTR";
        machine.helpstrm() << "\"str\" => ";
        return;
    }
    if (machine.stack_.size() < 1)
        throw std::runtime_error("PRINT: Requires object at L0");

    ObjectPtr optr;
    machine.pop(optr);

    std::cout << ToStr(machine, optr) << std::endl;
}

void PROMPT(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "PROMPT: Prompt for user input";
        machine.helpstrm() << "\"prompt\" => \"response\"";
        return;
    }
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

void PREAD(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "PREAD: Capture process output into a list";
        machine.helpstrm() << "\"command line\" opts PREAD => [dstlist]";
        machine.helpstrm() << "opts: --limit=n  Read a maximum of n lines of data";
        return;
    }
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

void PWRITE(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "PWRITE: Write object at L1 to the commandline on L0";
        machine.helpstrm() << "\"obj\" \"command line\" PWRITE =>";
        return;
    }
   stack_required(machine, "PWRITE", 2);

   ObjectPtr data;
   std::string cmd;

   machine.pop(cmd);
   machine.pop(data);

   FILE *fp = popen(cmd.c_str(), "w");
   if (fp)
   {
       if (data->type == OBJECT_LIST)
       {
            List *lp = (List *)data.get();
            for (ObjectPtr optr : lp->items)
            {
                std::string s = ToStr(machine, optr);
                fputs(s.c_str(), fp);
                fputs("\n", fp);
            }
       }
       else
       {
           String *sp = (String *)data.get();
           fputs(sp->value.c_str(), fp);
           fputs("\n", fp);
       }
       pclose(fp);
   }
}

void FWRITE(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "FWRITE: Write object at L1 to the file on L0";
        machine.helpstrm() << "\"obj\" \"filename\" FWRITE =>";
        return;
    }

   stack_required(machine, "FWRITE", 2);
   throw_required(machine, "FREAD", 0, OBJECT_STRING);

   ObjectPtr data;
   std::string file;

   machine.pop(file);
   machine.pop(data);

   FILE *fp = fopen(file.c_str(), "w");
   if (fp)
   {
       if (data->type == OBJECT_LIST)
       {
            List *lp = (List *)data.get();
            for (ObjectPtr optr : lp->items)
            {
                std::string s = ToStr(machine, optr);
                fputs(s.c_str(), fp);
                fputs("\n", fp);
            }
       }
       else
       {
           std::string s = ToStr(machine, data);
           String *sp = (String *)data.get();
           fputs(sp->value.c_str(), fp);
           fputs("\n", fp);
       }
       fclose(fp);
   }
}

void FREAD(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "FREAD: Capture a file into a list";
        machine.helpstrm() << "\"filename\" opts FREAD => [list]";
        machine.helpstrm() << "opts: --limit=n  Read a maximum of n lines of data";
        return;
    }
   stack_required(machine, "FREAD", 1);
   throw_required(machine, "FREAD", 0, OBJECT_STRING);

   std::string opt;
   std::string file;
   machine.pop(opt);
   int limit = std::numeric_limits<int>::max();
   while (strncmp(opt.c_str(), "--", 2) == 0)
   {
       if (strncmp(opt.c_str(), "--limit=", 8) == 0)
       {
            limit = std::stoi(&opt.c_str()[8]);
       }
       throw_required(machine, "FREAD", 0, OBJECT_STRING);
       machine.pop(opt);
   }
   file = opt;
   ListPtr ret = MakeList();

   FILE *fp = fopen(file.c_str(), "r");
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
       fclose(fp);
   }
   machine.push(ret);
}

void SYSTEM(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "SYSTEM or !: Execute the command at L0";
        machine.helpstrm() << "No output is captured";
        machine.helpstrm() << "\"command line\" SYSTEM =>";
        return;
    }
    stack_required(machine, "SYSTEM", 1);
    throw_required(machine, "SYSTEM", 0, OBJECT_STRING);
    std::string cmd;
    machine.pop(cmd);
    system(cmd.c_str());
}
