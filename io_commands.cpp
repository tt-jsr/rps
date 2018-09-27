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
#include "parser.h"

void PRINT(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "PRINT: Print the object at L0 according to the rules of TOSTR";
        machine.helpstrm() << "\"str\" => ";
        return;
    }
    stack_required(machine, "PRINT", 1);

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

    stack_required(machine, "PROMPT", 1);
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
   std::vector<std::string> args;
   int limit = std::numeric_limits<int>::max();

   GetArgs(machine, args);
   for(auto arg : args)
   {
       if (strncmp(arg.c_str(), "--limit=", 8) == 0)
       {
            limit = std::stoi(&arg.c_str()[8]);
       }
   }
   throw_required(machine, "PREAD", 0, OBJECT_STRING);
   machine.pop(cmd);

   FILE *fp = popen(cmd.c_str(), "r");
   if (fp)
   {
       ListPtr ret = MakeList();
       char buf[10240];
       for (int count = 0; count < limit && !feof(fp); ++count)
       {
           if (bInterrupt)
               break;
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
       machine.push(ret);
   }
   else
   {
       std::stringstream strm;
       strm << "Failed to open pipe " << cmd.c_str() << " for reading";
       throw std::runtime_error(strm.str().c_str());
   }
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
                if (bInterrupt)
                    break;
                std::string s = ToStr(machine, optr);
                fputs(s.c_str(), fp);
                fputs("\n", fp);
            }
       }
       else
       {
           std::string s = ToStr(machine, data);
           fputs(s.c_str(), fp);
           fputs("\n", fp);
       }
       pclose(fp);
   }
   else
   {
       std::stringstream strm;
       strm << "Failed to open pipe " << cmd.c_str() << " for writing";
       throw std::runtime_error(strm.str().c_str());
   }
}

void FWRITE(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "FWRITE: Write list at L1 to the file on L0";
        machine.helpstrm() << "[list] \"filename\" FWRITE =>";
        return;
    }

   stack_required(machine, "FWRITE", 2);
   throw_required(machine, "FWRITE", 0, OBJECT_STRING);
   throw_required(machine, "FWRITE", 1, OBJECT_LIST);

   ListPtr data;
   std::string file;

   machine.pop(file);
   machine.pop(data);

   FILE *fp = fopen(file.c_str(), "w");
   if (fp)
   {
        List *lp = (List *)data.get();
        for (ObjectPtr optr : lp->items)
        {
            if (bInterrupt)
                break;
            std::string s = ToStr(machine, optr);
            fputs(s.c_str(), fp);
            fputs("\n", fp);
        }
   }
   else
   {
       std::stringstream strm;
       strm << "Failed to open " << file.c_str() << " for writing";
       throw std::runtime_error(strm.str().c_str());
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
   int limit = std::numeric_limits<int>::max();

   std::vector<std::string> args;
   GetArgs(machine, args);
   for (auto& arg : args)
   {
       if (strncmp(arg.c_str(), "--limit=", 8) == 0)
       {
            limit = std::stoi(&arg.c_str()[8]);
       }
   }
   throw_required(machine, "FREAD", 0, OBJECT_STRING);
   machine.pop(file);

   FILE *fp = fopen(file.c_str(), "r");
   if (fp)
   {
       char buf[10240];
       ListPtr ret = MakeList();
       for (int count = 0; count < limit && !feof(fp); ++count)
       {
           if (bInterrupt)
               break;
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
       machine.push(ret);
   } 
   else
   {
       std::stringstream strm;
       strm << "Failed to open " << file.c_str() << " for reading";
       throw std::runtime_error(strm.str().c_str());
   }
}

void FSAVE(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "FSAVE: Write obj at L1 to the file on L0";
        machine.helpstrm() << "obj \"filename\" FSAVE =>";
        machine.helpstrm() << "FSAVE/FRESTORE is suitable for writing any object to a file for editing or";
        machine.helpstrm() << "archiving.";
        machine.helpstrm() << "See also: FRESTORE";

        return;
    }

   stack_required(machine, "FSAVE", 2);
   throw_required(machine, "FSAVE", 0, OBJECT_STRING);

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
            fputs("[", fp);
            fputs("\n", fp);
            for (ObjectPtr optr : lp->items)
            {
                if (bInterrupt)
                    break;
                std::string s = ToStr(machine, optr);
                fputs(s.c_str(), fp);
                fputs("\n", fp);
            }
            fputs("]", fp);
        }
        else
        {
            std::string s = ToStr(machine, data);
            fputs(s.c_str(), fp);
            fputs("\n", fp);
        }
        fclose(fp);
   }
   else
   {
       std::stringstream strm;
       strm << "Failed to open " << file.c_str() << " for writing";
       throw std::runtime_error(strm.str().c_str());
   }
}

void FRESTORE(Machine& machine)
{
    if (machine.help)
    {
        machine.helpstrm() << "FRESTORE: Restore a saved Object";
        machine.helpstrm() << "\"filename\" FRESTORE => obj";
        machine.helpstrm() << "Restore an object save with FSAVE";
        return;
    }
    stack_required(machine, "FRESTORE", 1);
    throw_required(machine, "FRESTORE", 0, OBJECT_STRING);

    std::string file;
    machine.pop(file);
    std::string data;

    FILE *fp = fopen(file.c_str(), "r");
    if (fp)
    {
        char buf[10240];
        while (!feof(fp))
        {
            if (fgets(buf, sizeof(buf), fp))
            {
                data.append(buf);
            }
        }
        fclose(fp);
        Parser parser(machine);

        std::stringstream strm;
        strm.str(data);
        Source src(strm);
        src.interactive = false;
        src.prompt = "> ";

        while (true)
        {
            try
            {
                parser.Parse(machine, src);
                return;
            }
            catch (std::runtime_error& e)
            {
                std::cout << e.what() << std::endl;
            }
        }
    }
   else
   {
       std::stringstream strm;
       strm << "Failed to open " << file.c_str() << " for reading";
       throw std::runtime_error(strm.str().c_str());
   }
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
