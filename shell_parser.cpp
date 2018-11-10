#include <vector>
#include <unordered_map>
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <readline/readline.h>
#include <readline/history.h>
#include "object.h"
#include "module.h"
#include "machine.h"
#include "token.h"
#include "parser.h"
#include "commands.h"
#include "utilities.h"
#include "shell.h"

namespace rps
{

std::string word(Source& src)
{
    std::string s;
    auto it = src.it;
    while (*it == ' ') 
        ++it;

    while (*it != ' ' && *it != '\n') 
    {
        s.push_back(*it);
        ++it;
    }
    return s;
}

ShellParser::ShellParser(Machine&)
{
}

void ShellParser::Parse(Machine& machine, Source& src, std::string& status)
{
    std::string savePrompt = src.prompt;
    src.prompt = "$ ";
    status.clear();
    while (!src.istrm.eof())
    {
        src.it = src.line.end();
        src.Read();
        std::string w = word(src);
        src.it = src.line.end();
        if (w == "rpn")
        {
            status = "rpn";
            return;
        }
        if (w == "exit")
        {
            status.clear();
            return;
        }
        Parse(machine, src.line);
    }
    src.prompt = savePrompt;
}

void ShellParser::Parse(Machine& machine, const std::string& commandLine)
{
    std::string word;
    for (auto it = commandLine.begin(); it != commandLine.end(); ++it)
    {
        if (*it == '%')
        {
            std::stringstream strm;
            std::string spec;
            ++it;
            if (*it == '{')
            {
                ++it;
                while(*it != '\n' && *it != '}')
                {
                    spec.push_back(*it);
                    ++it;
                }
                if(*it != '\n')
                    ++it;
            }
            else
            {
                while(it != commandLine.end() && (isalnum(*it) || *it == '.' || *it == '_'))
                {
                    spec.push_back(*it);
                    ++it;
                }
                --it;
            }
            if (isdigit(spec[0]))
            {
                int n = strtol(spec.c_str(), nullptr, 10);
                if (n >= machine.stack_.size())
                    throw std::runtime_error("FORMAT: Stack out of bounds");
                strm << ToStr(machine, machine.peek(n));
            }
            else 
            {
                ObjectPtr optr;
                try 
                {
                    RCL(machine, spec, optr);
                    strm << ToStr(machine, optr);
                }
                catch(std::exception&)
                {
                    strm << spec;
                }
            }
            word += strm.str();
            //std::cout << "=== word: " << word <<std::endl;
        }
        else if (*it == ' ' || *it == '\t')
        {
            if (!word.empty())
            {
                PushWord(machine, word.c_str());
                word.clear();
            }
        }
        else if (*it == '\"')
        {
            word.push_back(*it);  // we need to " char
            ++it;
            while (it != commandLine.end() && *it != '\"')
            {
                if (*it == '\\')
                {
                    ++it;
                    if (*it == 'n')
                        word.push_back('\n');
                    else if (*it == 't')
                        word.push_back('\t');
                    else
                        word.push_back(*it);
                }
                else
                    word.push_back(*it);
                ++it;
            }
            word.push_back(*it);
            PushWord(machine, word.c_str());
            word.clear();
        }
        else if (*it == '\\')
        {
            ++it;
            if (*it == 'n')
                word.push_back('\n');
            else if (*it == 't')
                word.push_back('\t');
            else
                word.push_back(*it);
        }
        else if (*it == '|')
        {
            if (!word.empty())
            {
                PushWord(machine, word.c_str());
                word.clear();
            }
            PushBar(machine);
        }
        else if (*it == '>')
        {
            if (!word.empty())
            {
                PushWord(machine, word.c_str());
                word.clear();
            }
            if (*(it+1) == '>')
            {
                ++it;
                PushGTGT(machine);
            }
            else
                PushGT(machine);
        }
        else if (*it == '<')
        {
            if (!word.empty())
            {
                PushWord(machine, word.c_str());
                word.clear();
            }
            PushLT(machine);
        }
        else if (*it == ';')
        {
            if (!word.empty())
            {
                PushWord(machine, word.c_str());
                word.clear();
            }
            PushSemi(machine);
        }
        else if (*it == '&')
        {
            if (!word.empty())
            {
                PushWord(machine, word.c_str());
                word.clear();
            }
            PushAmp(machine);
        }
        else if (*it == '!')
        {
            if (!word.empty())
            {
                PushWord(machine, word.c_str());
                word.clear();
            }
            PushBang(machine);
        }
        else if (*it == '\n')
        {
            if (!word.empty())
            {
                PushWord(machine, word.c_str());
                word.clear();
            }
            PushNL(machine);
        }
        else
            word.push_back(*it);
    }
}

} // namespace rps

