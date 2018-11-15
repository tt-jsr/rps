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

void SkipWhitespace(Source& src)
{
    while(!src.istrm.eof())
    {
        while (src.it != src.line.end())
        {
            if (*src.it == ' ' || *src.it == '\t')
                ++src.it;
            else
                return;
        }
        src.Read();
    }
}

bool CollectDelimited(Source& src, std::string& out, char delim)
{
    out.clear();
    assert(*src.it == delim);
    ++src.it;
    while(!src.istrm.eof())
    {
        while (src.it != src.line.end())
        {
            if (*src.it == '\\')
            {
                ++src.it;
                if (*src.it == 'n')
                    out.push_back('\n');
                else if (*src.it == 't')
                    out.push_back('\t');
                else
                    out.push_back(*src.it);
            }
            else if (*src.it == delim)
            {
                ++src.it;
                return true;
            }
            else
                out.push_back(*src.it);
            ++src.it;
        }
        src.Read();
    }
    if (out.size())
        return true;
    return false;
}

bool CollectIdentifier(Source& src, std::string& out)
{
    out.clear();

    if (*src.it == '(' and *(src.it+1) == ')')
    {
        out = "()";
        src.it += 2;
        return true;
    }
    if (*src.it == '-' && isdigit(*(src.it+1)))
        return false;
    if (isdigit(*src.it))
        return false;
    while (src.it != src.line.end())
    {
        if (isalnum(*src.it))
            out.push_back(*src.it);
        else if (isdigit(*src.it))
            out.push_back(*src.it);
        else if (*src.it == '.')
            out.push_back(*src.it);
        else if (*src.it == '_')
            out.push_back(*src.it);
        else if (*src.it == '/')
            out.push_back(*src.it);
        else if (*src.it == '-')
            out.push_back(*src.it);
        else
            break;
        ++src.it;
    }
    if (out.size())
        return true;
    return false;
}

bool CollectInteger(Source& src, std::string& out)
{
    out.clear();
    if (*src.it == '-')
    {
        ++src.it;
        if (isdigit(*src.it) == false)
        {
            --src.it;
            return false;
        }
        out.push_back('-');
    }
    while (src.it != src.line.end())
    {
        if (isdigit(*src.it))
            out.push_back(*src.it);
        else
            break;
        ++src.it;
    }
    if (out.size())
        return true;
    return false;
}

bool RPNParser::GetObject(Machine& machine, Source& src, ObjectPtr& optr)
{
    std::string out;
    SkipWhitespace(src);
    if(src.iseof())
    {
        return false;
    }

    if (*src.it == '\n')
    {
        optr.reset(new Token(TOKEN_EOL, "\n"));
        ++src.it;
        return true;
    }
    if (*src.it == '\"')
    {
        CollectDelimited(src, out, '\"');
        String *sp = new String(out);
        optr.reset(sp);
        return true;
    }
    if (*src.it == '\'')
    {
        CollectDelimited(src, out, '\'');
        String *sp = new String(out);
        optr.reset(sp);
        return true;
    }
    if (*src.it == '[')
    {
        optr.reset(new Token(TOKEN_START_LIST, "["));
        ++src.it;
        return true;
    }
    if (*src.it == '{')
    {
        optr.reset(new Token(TOKEN_START_MAP, "{"));
        ++src.it;
        return true;
    }
    if (*src.it == ']')
    {
        optr.reset(new Token(TOKEN_END_LIST, "]"));
        ++src.it;
        return true;
    }
    if (*src.it == '}')
    {
        optr.reset(new Token(TOKEN_END_MAP, "}"));
        ++src.it;
        return true;
    }
    if (*src.it == '<')
    {
        ++src.it;
        if (*src.it != '<')
            throw std::runtime_error("Expected \'<\' in input");
        optr.reset(new Token(TOKEN_START_PROGRAM, "<<"));
        ++src.it;
        return true;
    }
    if (*src.it == '>')
    {
        ++src.it;
        if (*src.it != '>')
            throw std::runtime_error("Expected \'>\' in input");
        ++src.it;
        optr.reset(new Token(TOKEN_END_PROGRAM, ">>"));
        return true;
    }
    if (*src.it == '#')
    {
        src.it = src.line.end();
        optr.reset(new Token(TOKEN_COMMENT, "#"));
        return true;
    }

    if (CollectIdentifier(src, out))
    {
        if (out == "None")
            optr.reset(new None());
        else if (out == "EXIT")
            optr.reset(new Token(TOKEN_EXIT, "EXIT"));
        else if (out == "IF")
            optr.reset(new Token(TOKEN_IF, "IF"));
        else if (out == "THEN")
            optr.reset(new Token(TOKEN_THEN, "THEN"));
        else if (out == "ELSE")
            optr.reset(new Token(TOKEN_ELSE, "ELSE"));
        else if (out == "ENDIF")
            optr.reset(new Token(TOKEN_ENDIF, "ENDIF"));
        else if (out == "FOR")
            optr.reset(new Token(TOKEN_FOR, "FOR"));
        else if (out == "ENDFOR")
            optr.reset(new Token(TOKEN_ENDFOR, "ENDFOR"));
        else if (out == "WHILE")
            optr.reset(new Token(TOKEN_WHILE, "WHILE"));
        else if (out == "REPEAT")
            optr.reset(new Token(TOKEN_REPEAT, "REPEAT"));
        else if (out == "ENDWHILE")
            optr.reset(new Token(TOKEN_ENDWHILE, "ENDWHILE"));
        else if (out == "SHELL")
            optr.reset(new Token(TOKEN_SHELL, "SHELL"));
        else
        {
            auto it = machine.commands.find(out);
            if (it != machine.commands.end())
            {
                optr = it->second;
                return true;
            }
            optr.reset(new String(out));
        }
        return true;
    }
    if (CollectInteger(src, out))
    {
        optr.reset(new Integer(strtol(out.c_str(), nullptr, 10)));
        return true;
    }
    std::string s;
    s.push_back(*src.it);
    optr.reset(new Token(TOKEN_INVALID, s));
    ++src.it;
    return false;
}

void RPNParser::ParseIf(Machine& machine, IfPtr& ifptr, Source& src)
{
    ObjectPtr optr;
    std::vector<ObjectPtr> *pVec = &ifptr->cond;
    while(GetObject(machine, src, optr))
    {
        if (bInterrupt)
            return;
        if (optr->IsToken(TOKEN_ENDIF))
        {
            return;
        }
        else if (optr->IsToken(TOKEN_THEN))
        {
            pVec = &ifptr->then;
            src.prompt = "THEN: ";
        }
        else if (optr->IsToken(TOKEN_ELSE))
        {
            pVec = &ifptr->els;
            src.prompt = "ELSE: ";
        }
        else if (optr->IsToken(TOKEN_IF))
        {
            IfPtr ifp;
            ifp.reset(new If());
            std::string savePrompt = src.prompt;
            src.prompt = "IF: ";
            ParseIf(machine, ifp, src);    // recurse
            src.prompt = savePrompt;
            optr = ifp;
            pVec->push_back(optr);           
        }
        else if (optr->IsToken(TOKEN_FOR))
        {
            ForPtr forptr;
            forptr.reset(new For());
            std::string savePrompt = src.prompt;
            src.prompt = "FOR: ";
            ParseFor(machine, forptr, src); 
            src.prompt = savePrompt;
            optr = forptr;
            pVec->push_back(optr);           
        }
        else if (optr->IsToken(TOKEN_WHILE))
        {
            WhilePtr whileptr;
            whileptr.reset(new While());
            std::string savePrompt = src.prompt;
            src.prompt = "WHILE: ";
            ParseWhile(machine, whileptr, src); 
            src.prompt = savePrompt;
            optr = whileptr;
            pVec->push_back(optr);           
        }
        else if (optr->IsToken(TOKEN_START_PROGRAM))
        {
            ProgramPtr pptr;
            pptr.reset(new Program());
            std::string savePrompt = src.prompt;
            src.prompt = ">> ";
            if (enclosingProgram)
                pptr->enclosingProgram = enclosingProgram;
            enclosingProgram = pptr;
            ParseProgram(machine, pptr, src);  
            enclosingProgram = pptr->enclosingProgram;
            src.prompt = savePrompt;
            optr = pptr;
            pVec->push_back(optr);           
        }
        else if (optr->IsToken(TOKEN_START_LIST))
        {
            ListPtr lp;
            lp.reset(new List());
            std::string savePrompt = src.prompt;
            src.prompt = "[] ";
            ParseList(machine, lp, src);    // recurse
            src.prompt = savePrompt;
            optr = lp;
            pVec->push_back(optr);           
        }
        else
            pVec->push_back(optr);           
    }
}

void RPNParser::ParseFor(Machine& machine, ForPtr& forptr, Source& src)
{
    ObjectPtr optr;
    while(GetObject(machine, src, optr))
    {
        if (bInterrupt)
            return;
        if (optr->IsToken(TOKEN_ENDFOR))
        {
            return;
        }
        else if (optr->IsToken(TOKEN_FOR))
        {
            ForPtr forp;
            forp.reset(new For());
            std::string savePrompt = src.prompt;
            src.prompt = "FOR: ";
            ParseFor(machine, forp, src);    // recurse
            src.prompt = savePrompt;
            optr = forp;
        }
        else if (optr->IsToken(TOKEN_IF))
        {
            IfPtr ifp;
            ifp.reset(new If());
            std::string savePrompt = src.prompt;
            src.prompt = "IF: ";
            ParseIf(machine, ifp, src);    // recurse
            src.prompt = savePrompt;
            optr = ifp;
        }
        else if (optr->IsToken(TOKEN_START_PROGRAM))
        {
            ProgramPtr pptr;
            pptr.reset(new Program());
            std::string savePrompt = src.prompt;
            src.prompt = ">> ";
            if (enclosingProgram)
                pptr->enclosingProgram = enclosingProgram;
            enclosingProgram = pptr;
            ParseProgram(machine, pptr, src);    // recurse
            enclosingProgram = pptr->enclosingProgram;
            src.prompt = savePrompt;
            optr = pptr;
        }
        else if (optr->IsToken(TOKEN_START_LIST))
        {
            ListPtr lp;
            lp.reset(new List());
            std::string savePrompt = src.prompt;
            src.prompt = "[] ";
            ParseList(machine, lp, src);    // recurse
            src.prompt = savePrompt;
            optr = lp;
        }
        //std::cout << "=== ParseFor: " << ToStr(machine, optr) << std::endl;
        forptr->program.push_back(optr);           
    }
}

void RPNParser::ParseWhile(Machine& machine, WhilePtr& whileptr, Source& src)
{
    ObjectPtr optr;
    std::vector<ObjectPtr> *pVec = &whileptr->cond;
    while(GetObject(machine, src, optr))
    {
        if (bInterrupt)
            return;
        if (optr->IsToken(TOKEN_ENDWHILE))
        {
            return;
        }
        else if (optr->IsToken(TOKEN_REPEAT))
        {
            pVec = &whileptr->program;
            src.prompt = "REPEAT: ";
        }
        else if (optr->IsToken(TOKEN_WHILE))
        {
            WhilePtr whilep;
            whilep.reset(new While());
            std::string savePrompt = src.prompt;
            src.prompt = "WHILE: ";
            ParseWhile(machine, whilep, src);    // recurse
            src.prompt = savePrompt;
            optr = whilep;
            pVec->push_back(optr);
        }
        else if (optr->IsToken(TOKEN_FOR))
        {
            ForPtr forp;
            forp.reset(new For());
            std::string savePrompt = src.prompt;
            src.prompt = "FOR: ";
            ParseFor(machine, forp, src);    // recurse
            src.prompt = savePrompt;
            optr = forp;
            pVec->push_back(optr);
        }
        else if (optr->IsToken(TOKEN_IF))
        {
            IfPtr ifp;
            ifp.reset(new If());
            std::string savePrompt = src.prompt;
            src.prompt = "IF: ";
            ParseIf(machine, ifp, src);    // recurse
            src.prompt = savePrompt;
            optr = ifp;
            pVec->push_back(optr);
        }
        else if (optr->IsToken(TOKEN_START_PROGRAM))
        {
            ProgramPtr pptr;
            pptr.reset(new Program());
            std::string savePrompt = src.prompt;
            src.prompt = ">> ";
            if (enclosingProgram)
                pptr->enclosingProgram = enclosingProgram;
            enclosingProgram = pptr;
            ParseProgram(machine, pptr, src);    // recurse
            enclosingProgram = pptr->enclosingProgram;
            src.prompt = savePrompt;
            optr = pptr;
            pVec->push_back(optr);
        }
        else if (optr->IsToken(TOKEN_START_LIST))
        {
            ListPtr lp;
            lp.reset(new List());
            std::string savePrompt = src.prompt;
            src.prompt = "[] ";
            ParseList(machine, lp, src);    // recurse
            src.prompt = savePrompt;
            optr = lp;
            pVec->push_back(optr);
        }
        else
            pVec->push_back(optr);           
    }
}

void RPNParser::ParseList(Machine& machine, ListPtr& lptr, Source& src)
{
    ObjectPtr optr;
    while(GetObject(machine, src, optr))
    {
        if (bInterrupt)
            return;
        if (optr->IsToken(TOKEN_END_LIST))
        {
            return;
        }
        else if (optr->IsToken(TOKEN_EOL))
            ;
        else if (optr->IsToken(TOKEN_START_PROGRAM))
        {
            ProgramPtr pptr;
            pptr.reset(new Program());
            src.prompt = ">> ";
            if (enclosingProgram)
                pptr->enclosingProgram = enclosingProgram;
            enclosingProgram = pptr;
            ParseProgram(machine, pptr, src);    // recurse
            enclosingProgram = pptr->enclosingProgram;
            src.prompt = "[] ";
            optr = pptr;
            lptr->items.push_back(optr);           
        }
        else if (optr->IsToken(TOKEN_START_LIST))
        {
            ListPtr lp;
            lp.reset(new List());
            ParseList(machine, lp, src);    // recurse
            optr = lp;
            lptr->items.push_back(optr);           
        }
        else
            lptr->items.push_back(optr);           
    }
}

void RPNParser::ParseProgram(Machine& machine, ProgramPtr& pptr, Source& src)
{
    ObjectPtr optr;
    pptr->module_name = machine.current_module_;
    while(GetObject(machine, src, optr))
    {
        if (bInterrupt)
            return;
        if (optr->IsToken(TOKEN_END_PROGRAM))
        {
            return;
        }
        else if (optr->IsToken(TOKEN_START_PROGRAM))
        {
            ProgramPtr pp;
            pp.reset(new Program());
            if (enclosingProgram)
                pp->enclosingProgram = enclosingProgram;
            enclosingProgram = pp;
            ParseProgram(machine, pp, src);    // recurse
            enclosingProgram = pp->enclosingProgram;
            optr = pp;
        }
        else if (optr->IsToken(TOKEN_START_LIST))
        {
            ListPtr lp;
            lp.reset(new List());
            src.prompt = "[] ";
            ParseList(machine, lp, src);    // recurse
            src.prompt = ">> ";
            optr = lp;
        }
        else if (optr->IsToken(TOKEN_IF))
        {
            IfPtr ifp;
            ifp.reset(new If());
            src.prompt = "IF: ";
            ParseIf(machine, ifp, src);    // recurse
            src.prompt = ">> ";
            optr = ifp;
        }
        else if (optr->IsToken(TOKEN_FOR))
        {
            ForPtr forptr;
            forptr.reset(new For());
            std::string savePrompt = src.prompt;
            src.prompt = "FOR: ";
            ParseFor(machine, forptr, src);    // recurse
            src.prompt = savePrompt;
            optr = forptr;
        }
        else if (optr->IsToken(TOKEN_WHILE))
        {
            WhilePtr whileptr;
            whileptr.reset(new While());
            std::string savePrompt = src.prompt;
            src.prompt = "WHILE: ";
            ParseWhile(machine, whileptr, src);    // recurse
            src.prompt = savePrompt;
            optr = whileptr;
        }
        pptr->program.push_back(optr);           
    }
}

void RPNParser::Parse(Machine& machine, Source& src, std::string& exit)
{
    exit.clear();
    src.prompt = "> ";
    while (!src.istrm.eof())
    {
        ObjectPtr optr;
        while(GetObject(machine, src, optr))
        {
            if (!optr)
                continue;
            if (optr->IsToken(TOKEN_EXIT))
            {
                return;
            }
            if (optr->IsToken(TOKEN_SHELL))
            {
                exit = "shell";
                return;
            }
            if (optr->IsToken(TOKEN_START_LIST))
            {
                src.prompt = "[] ";
                ListPtr lptr;
                lptr.reset(new List());
                ParseList(machine, lptr, src);
                src.prompt = "> ";
                optr = lptr;
            }
            if (optr->IsToken(TOKEN_START_PROGRAM))
            {
                src.prompt = ">> ";
                ProgramPtr pptr;
                pptr.reset(new Program());
                enclosingProgram = pptr;
                ParseProgram(machine, pptr, src);
                enclosingProgram.reset();
                src.prompt = "> ";
                optr = pptr;
            }

            if (optr->IsToken(TOKEN_FOR))
            {
                src.prompt = "FOR: ";
                ForPtr forptr;
                forptr.reset(new For());
                ParseFor(machine, forptr, src);
                src.prompt = "> ";
                try
                {
                    Execute(machine, forptr);
                }
                catch (std::exception& e)
                {
                    std::cout << e.what() << std::endl;
                }
            }
            if (optr->IsToken(TOKEN_WHILE))
            {
                src.prompt = "WHILE: ";
                WhilePtr whileptr;
                whileptr.reset(new While());
                ParseWhile(machine, whileptr, src);
                src.prompt = "> ";
                try
                {
                    Execute(machine, whileptr);
                }
                catch (std::exception& e)
                {
                    std::cout << e.what() << std::endl;
                }
            }

            else if (optr->IsToken(TOKEN_IF))
            {
                src.prompt = "IF: ";
                IfPtr ifptr;
                ifptr.reset(new If());
                ParseIf(machine, ifptr, src);
                src.prompt = "> ";
                try
                {
                    Execute(machine, ifptr);
                }
                catch (std::exception& e)
                {
                    std::cout << e.what() << std::endl;
                }
            }
            else if (optr->type == OBJECT_COMMAND)
            {
                try
                {
                    Execute(machine, optr);
                }
                catch (std::exception& e)
                {
                    std::cout << e.what() << std::endl;
                }
            }
            else if (optr->IsToken(TOKEN_EOL))
            {
                if (src.interactive )
                {
                    VIEW(machine, 4);
                }
            }
            else if (optr->type == OBJECT_STRING)
            {
                String *sp = (String *)optr.get();
                if (sp->get()[0] == '&')
                {
                    sp->set(sp->get().substr(1));
                }
                machine.push(optr);           
            }
            else if (optr->IsToken(TOKEN_COMMENT))
            {
                ;
            }
            else
            {
                machine.push(optr);           
            }
            bInterrupt = false;
        }
    }
}

RPNParser::RPNParser(Machine& machine)
{
    // Stack commands
    AddCommand(machine, "CLRSTK", &CLRSTK);
    Category(machine, "Stack", "CLRSTK");
    AddCommand(machine, "DROP", &DROP);
    Category(machine, "Stack", "DROP");
    AddCommand(machine, "DROPN", &DROPN);
    Category(machine, "Stack", "DROPN");
    AddCommand(machine, "SWAP", &SWAP);
    Category(machine, "Stack", "SWAP");
    AddCommand(machine, "DUP", &DUP);
    Category(machine, "Stack", "DUP");
    AddCommand(machine, "PICK", &PICK);
    Category(machine, "Stack", "PICK");
    AddCommand(machine, "ROLL", &ROLL);
    Category(machine, "Stack", "ROLL");
    AddCommand(machine, "ROLLD", &ROLLD);
    Category(machine, "Stack", "ROLLD");
    AddCommand(machine, "DEPTH", &DEPTH);
    Category(machine, "Stack", "DEPTH");
    AddCommand(machine, "VIEW", &VIEW);
    Category(machine, "Stack", "VIEW");

    // Logical operators
    AddCommand(machine, "EQ", &EQ);
    Category(machine, "Logical", "EQ");
    AddCommand(machine, "NEQ", &NEQ);
    Category(machine, "Logical", "NEQ");
    AddCommand(machine, "LT", &LT);
    Category(machine, "Logical", "LT");
    AddCommand(machine, "LTEQ", &LTEQ);
    Category(machine, "Logical", "LTEQ");
    AddCommand(machine, "GT", &GT);
    Category(machine, "Logical", "GT");
    AddCommand(machine, "GTEQ", &GTEQ);
    Category(machine, "Logical", "GTEQ");
    AddCommand(machine, "AND", &AND);
    Category(machine, "Logical", "AND");
    AddCommand(machine, "OR", &OR);
    Category(machine, "Logical", "OR");
    AddCommand(machine, "NOT", &NOT);
    Category(machine, "Logical", "NOT");

    // Variable commands
    AddCommand(machine, "STO", &STO);
    Category(machine, "Variable", "STO");
    AddCommand(machine, "RCL", &RCL);
    Category(machine, "Variable", "RCL");
    AddCommand(machine, "STOL", &STOL);
    Category(machine, "Variable", "STOL");
    AddCommand(machine, "RCLL", &RCLL);
    Category(machine, "Variable", "RCLL");
    AddCommand(machine, "RCLA", &RCLA);
    AddCommand(machine, "%", &RCLA);
    Category(machine, "Variable", "RCLA");
    AddCommand(machine, "VARNAMES", &VARNAMES);
    Category(machine, "Variable", "VARNAMES");
    AddCommand(machine, "VARS", &VARS);
    Category(machine, "Variable", "VARS");
    AddCommand(machine, "VARTYPES", &VARTYPES);
    Category(machine, "Variable", "VARTYPES");
    AddCommand(machine, "REGISTER", &REGISTER);
    Category(machine, "Variable", "REGISTER");
    AddCommand(machine, "UNREGISTER", &UNREGISTER);
    Category(machine, "Variable", "UNREGISTER");

    // Math commands
    AddCommand(machine, "ADD", &ADD);
    Category(machine, "Math", "ADD");
    AddCommand(machine, "SUB", &SUB);
    Category(machine, "Math", "SUB");
    AddCommand(machine, "MUL", &MUL);
    Category(machine, "Math", "MUL");
    AddCommand(machine, "DIV", &DIV);
    Category(machine, "Math", "DIV");
    AddCommand(machine, "INC", &INC);
    Category(machine, "Math", "INC");
    AddCommand(machine, "DEC", &DEC);
    Category(machine, "Math", "DEC");

    // Control commands
    AddCommand(machine, "IFT", &IFT);
    Category(machine, "Control", "IFT");
    AddCommand(machine, "IFTE", &IFTE);
    Category(machine, "Control", "IFTE");
    AddCommand(machine, "TRYCATCH", &TRYCATCH);
    Category(machine, "Control", "TRYCATCH");

    // List commands
    AddCommand(machine, "GET", &GET);
    Category(machine, "List", "GET");
    AddCommand(machine, "SUBLIST", &SUBLIST);
    Category(machine, "List", "SUBLIST");
    AddCommand(machine, "APPEND", &APPEND);
    Category(machine, "List", "APPEND");
    AddCommand(machine, "ERASE", &ERASE);
    Category(machine, "List", "ERASE");
    AddCommand(machine, "CLEAR", &CLEAR);
    Category(machine, "List", "CLEAR");
    AddCommand(machine, "LINSERT", &LINSERT);
    Category(machine, "List", "LINSERT");
    AddCommand(machine, "INSERT", &INSERT);
    AddCommand(machine, "SIZE", &SIZE);
    Category(machine, "List", "SIZE");
    AddCommand(machine, "FIRST", &FIRST);
    Category(machine, "List", "FIRST");
    AddCommand(machine, "SECOND", &SECOND);
    Category(machine, "List", "SECOND");
    AddCommand(machine, "TOLIST", &TOLIST);
    Category(machine, "List", "TOLIST");
    AddCommand(machine, "FROMLIST", &FROMLIST);
    Category(machine, "List", "FROMLIST");
    AddCommand(machine, "CREATELIST", &CREATELIST);
    Category(machine, "List", "CREATELIST");
    AddCommand(machine, "HEAD", &HEAD);
    Category(machine, "List", "HEAD");
    AddCommand(machine, "UNIQUE", &UNIQUE);
    Category(machine, "List", "UNIQUE");
    AddCommand(machine, "REVERSE", &REVERSE);
    Category(machine, "List", "REVERSE");
    AddCommand(machine, "ZIP", &ZIP);
    Category(machine, "List", "ZIP");
    AddCommand(machine, "UNZIP", &UNZIP);
    Category(machine, "List", "UNZIP");

    // Map commands
    Category(machine, "Map", "ERASE");
    Category(machine, "Map", "CLEAR");
    AddCommand(machine, "MINSERT", &MINSERT);
    Category(machine, "Map", "MINSERT");
    Category(machine, "Map", "SIZE");
    AddCommand(machine, "FIND", &FIND);
    Category(machine, "Map", "FIND");
    AddCommand(machine, "TOMAP", &TOMAP);
    Category(machine, "Map", "TOMAP");
    AddCommand(machine, "FROMMAP", &FROMMAP);
    Category(machine, "Map", "FROMMAP");
    AddCommand(machine, "CREATEMAP", &CREATEMAP);
    Category(machine, "Map", "CREATEMAP");
    AddCommand(machine, "KEYS", &KEYS);
    Category(machine, "Map", "KEYS");
    AddCommand(machine, "VALUES", &VALUES);
    Category(machine, "Map", "VALUES");

    // Functional
    AddCommand(machine, "APPLY", &APPLY);
    Category(machine, "Functional", "APPLY");
    AddCommand(machine, "APPLY1", &APPLY1);
    Category(machine, "Functional", "APPLY1");
    AddCommand(machine, "FILTER", &FILTER);
    Category(machine, "Functional", "FILTER");
    AddCommand(machine, "FILTER1", &FILTER1);
    Category(machine, "Functional", "FILTER1");
    AddCommand(machine, "MAP", &MAP);
    Category(machine, "Functional", "MAP");
    AddCommand(machine, "MAP1", &MAP1);
    Category(machine, "Functional", "MAP1");
    AddCommand(machine, "REDUCE", &REDUCE);
    Category(machine, "Functional", "REDUCE");

    // Execution commands
    AddCommand(machine, "EVAL", &EVAL);
    Category(machine, "Execution", "EVAL");
    AddCommand(machine, "CALL", &CALL);
    AddCommand(machine, "()", &CALL);
    Category(machine, "Execution", "CALL");
    AddCommand(machine, "SYSTEM", &SYSTEM);
    Category(machine, "Execution", "SYSTEM");
    AddCommand(machine, "INTERRUPT", &INTERRUPT);
    Category(machine, "Execution", "INTERRUPT");
    AddCommand(machine, "ALIAS", &ALIAS);
    Category(machine, "Execution", "ALIAS");


    // Environment
    AddCommand(machine, "NAMESPACES", &NAMESPACES);
    Category(machine, "Environment", "NAMESPACES");
    AddCommand(machine, "SETNS", &SETNS);
    Category(machine, "Environment", "SETNS");
    AddCommand(machine, "GETNS", &GETNS);
    Category(machine, "Environment", "GETNS");
    AddCommand(machine, "CD", &CD);
    Category(machine, "Environment", "CD");
    AddCommand(machine, "PWD", &PWD);
    Category(machine, "Environment", "PWD");
    AddCommand(machine, "HELP", &HELP);
    Category(machine, "Environment", "HELP");
    AddCommand(machine, "GETPROPERTY", &GETPROPERTY);
    Category(machine, "Environment", "GETPROPERTY");
    AddCommand(machine, "SETPROPERTY", &SETPROPERTY);
    Category(machine, "Environment", "SETPROPERTY");
    AddCommand(machine, "LISTPROPERTIES", &LISTPROPERTIES);
    Category(machine, "Environment", "LISTPROPERTIES");
    AddCommand(machine, "IMPORT", &IMPORT);
    Category(machine, "Environment", "IMPORT");

    // String
    AddCommand(machine, "FORMAT", &FORMAT);
    Category(machine, "String", "FORMAT");
    AddCommand(machine, "CAT", &CAT);
    Category(machine, "String", "CAT");
    AddCommand(machine, "JOIN", &JOIN);
    Category(machine, "String", "JOIN");
    AddCommand(machine, "SUBSTR", &SUBSTR);
    Category(machine, "String", "SUBSTR");
    AddCommand(machine, "SUBSTRPOS", &SUBSTRPOS);
    Category(machine, "String", "SUBSTRPOS");
    AddCommand(machine, "STRFIND", &STRFIND);
    Category(machine, "String", "STRFIND");
    AddCommand(machine, "STRFINDEND", &STRFINDEND);
    Category(machine, "String", "STRFINDEND");
    AddCommand(machine, "STRCMP", &STRCMP);
    Category(machine, "String", "STRCMP");
    AddCommand(machine, "STRNCMP", &STRNCMP);
    Category(machine, "String", "STRNCMP");
    AddCommand(machine, "SPLIT", &SPLIT);
    Category(machine, "String", "SPLIT");
    Category(machine, "String", "SIZE");
    AddCommand(machine, "STRBEGIN", &STRBEGIN);
    Category(machine, "String", "STRBEGIN");
    AddCommand(machine, "STREND", &STREND);
    Category(machine, "String", "STREND");
    AddCommand(machine, "STRHAS", &STRHAS);
    Category(machine, "String", "STRHAS");
    AddCommand(machine, "STRCSPN", &STRCSPN);
    Category(machine, "String", "STRCSPN");
    Category(machine, "String", "CLEAR");

    // Types
    AddCommand(machine, "TOINT", &TOINT);
    Category(machine, "Types", "TOINT");
    AddCommand(machine, "TOSTR", &TOSTR);
    Category(machine, "Types", "TOSTR");
    AddCommand(machine, "TYPE", &TYPE);
    Category(machine, "Types", "TYPE");
    AddCommand(machine, "CLONE",  &CLONE);
    Category(machine, "Types", "CLONE");

    // IO
    AddCommand(machine, "PRINT", &PRINT);
    Category(machine, "IO", "PRINT");
    AddCommand(machine, "PROMPT", &PROMPT);
    Category(machine, "IO", "PROMPT");
    AddCommand(machine, "PREAD", &PREAD);
    Category(machine, "IO", "PREAD");
    AddCommand(machine, "FREAD", &FREAD);
    Category(machine, "IO", "FREAD");
    AddCommand(machine, "PWRITE", &PWRITE);
    Category(machine, "IO", "PWRITE");
    AddCommand(machine, "FWRITE", &FWRITE);
    Category(machine, "IO", "FWRITE");
    AddCommand(machine, "FSAVE", &FSAVE);
    Category(machine, "IO", "FSAVE");
    AddCommand(machine, "FRESTORE", &FRESTORE);
    Category(machine, "IO", "FRESTORE");
}

/********************************************************/
Source::Source(std::istream& is)
:istrm(is)
, it(line.end())
, interactive(false)
, lineno(0)
{
}

bool Source::iseof()
{
    return istrm.eof();
}

void Source::Read()
{
    if (interactive)
    {
        if (it == line.end())
        {
            char *pLine = readline(prompt.c_str());
            add_history(pLine);
            line = pLine;
            free(pLine);
            line += "\n";
            ++lineno;
            it = line.begin();
        }
    }
    else
    {
        if (istrm.eof())
        {
            it = line.end();
            return;
        }
        if (it == line.end())
        {
            if (interactive)
            {
                std::cout << prompt << std::flush;
            }
            getline(istrm, line);
            line += "\n";
            ++lineno;
            it = line.begin();
        }
    }
}

} // namespace rps

