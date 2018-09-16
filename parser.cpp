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

void CollectQuoted(Source& src, Token& token)
{
    assert(*src.it == '\"');
    ++src.it;
    while(!src.istrm.eof())
    {
        while (src.it != src.line.end())
        {
            if (*src.it == '\\')
            {
                ++src.it;
                token.value.push_back(*src.it);
            }
            else
            {
                if (*src.it == '\"')
                {
                    ++src.it;
                    return;
                }
                token.value.push_back(*src.it);
            }
            if (src.it != src.line.end())
                ++src.it;
        }
        src.Read();
    }
}

void CollectIdentifier(Source& src, Token& token)
{
    while (src.it != src.line.end())
    {
        if (isalnum(*src.it))
            token.value.push_back(*src.it);
        else if (isdigit(*src.it))
            token.value.push_back(*src.it);
        else if (*src.it == '.' || *src.it == '_' || *src.it == '-')
            token.value.push_back(*src.it);
        else if (*src.it == ' ' || *src.it == ';')
            return;
        else if (*src.it == '#')
        {
            src.it = src.line.end();
            return;
        }
        else
        {
            std::stringstream strm;
            strm << "Identifier received unexpected \'" << *src.it << "\'";
            throw std::runtime_error(strm.str().c_str());
        }
        ++src.it;
    }
}

void CollectInteger(Source& src, Token& token)
{
    while (src.it != src.line.end())
    {
        if (isdigit(*src.it))
            token.value.push_back(*src.it);
        else if (*src.it == ' ' || *src.it == ';')
            return;
        else if (*src.it == '#')
        {
            src.it = src.line.end();
            return;
        }
        else
        {
            std::stringstream strm;
            strm << "Integer received unexpected \'" << *src.it << "\'";
            throw std::runtime_error(strm.str().c_str());
        }
        ++src.it;
    }
}
void GetToken(Source& src, Token& token)
{
    token.value.clear();
    SkipWhitespace(src);
    if(src.iseof())
    {
        token.token = TOKEN_EOF;
        return;
    }
    if (*src.it == '!')
    {
        token.value.push_back(*src.it);
        token.token = TOKEN_SYSTEM;
        ++src.it;
        return;
    }

    if (*src.it == ';')
    {
        token.value.push_back(*src.it);
        token.token = TOKEN_EOL;
        ++src.it;
        return;
    }
    if (*src.it == '\"')
    {
        CollectQuoted(src, token);
        token.token = TOKEN_STRING;
        return;
    }
    if (isdigit(*src.it) || *src.it == '-')
    {
        token.value.push_back(*src.it);
        ++src.it;
        CollectInteger(src, token);
        token.token = TOKEN_INTEGER;
        return;
    }
    if (isalpha(*src.it))
    {
        token.value.push_back(*src.it);
        ++src.it;
        CollectIdentifier(src, token);
        token.token = TOKEN_STRING;
        return;
    }
    if (*src.it == '[')
    {
        token.value.push_back(*src.it);
        ++src.it;
        token.token = TOKEN_START_LIST;
        return;
    }
    if (*src.it == '{')
    {
        token.value.push_back(*src.it);
        ++src.it;
        token.token = TOKEN_START_MAP;
        return;
    }
    if (*src.it == ']')
    {
        token.value.push_back(*src.it);
        ++src.it;
        token.token = TOKEN_END_LIST;
        return;
    }
    if (*src.it == '}')
    {
        token.value.push_back(*src.it);
        ++src.it;
        token.token = TOKEN_END_MAP;
        return;
    }
    if (*src.it == '<')
    {
        token.value.push_back(*src.it);
        ++src.it;
        if (*src.it != '<')
            throw std::runtime_error("Expected \'<\' in input");
        token.value.push_back(*src.it);
        ++src.it;
        token.token = TOKEN_START_PROGRAM;
        return;
    }
    if (*src.it == '>')
    {
        token.value.push_back(*src.it);
        ++src.it;
        if (*src.it != '>')
            throw std::runtime_error("Expected \'>\' in input");
        token.value.push_back(*src.it);
        ++src.it;
        token.token = TOKEN_END_PROGRAM;
        return;
    }
    if (*src.it == '#')
    {
        src.it = src.line.end();
        token.token = TOKEN_COMMENT;
    }
}

bool Parser::GetObject(Machine& machine, Source& src, ObjectPtr& optr)
{
    const char *rps_path = getenv("RPS_PATH");
    std::vector<std::string> dirs;
    dirs.push_back(".");
    if (rps_path)
        split(rps_path, dirs, ":");

again:
    Token token;
    do {
        GetToken(src, token);
    } while (token.token == TOKEN_COMMENT);
    if (token.token == TOKEN_EOF)
        return false;
    //std::cout << "===GetObject: " << token.token << ":\"" << token.value << "\"" << std::endl;

    auto it = machine.commands.find(token.value);
    if (it != machine.commands.end())
    {
        optr = it->second;
        return true;
    }

    if (token.token == TOKEN_INTEGER)
        optr.reset(new Integer(strtol(token.value.c_str(), nullptr, 10)));

    else if (token.token == TOKEN_START_PROGRAM)
        optr.reset(new Object(OBJECT_TOKEN, TOKEN_START_PROGRAM));
    else if (token.token == TOKEN_END_PROGRAM)
        optr.reset(new Object(OBJECT_TOKEN, TOKEN_END_PROGRAM));
    else if (token.token == TOKEN_START_LIST)
        optr.reset(new Object(OBJECT_TOKEN, TOKEN_START_LIST));
    else if (token.token == TOKEN_END_LIST)
        optr.reset(new Object(OBJECT_TOKEN, TOKEN_END_LIST));
    else if (token.token == TOKEN_START_MAP)
        optr.reset(new Object(OBJECT_TOKEN, TOKEN_START_MAP));
    else if (token.token == TOKEN_END_MAP)
        optr.reset(new Object(OBJECT_TOKEN, TOKEN_END_MAP));
    else if (token.token == TOKEN_SYSTEM)
        optr.reset(new Command(token.value, &SYSTEM));
    else if (token.token == TOKEN_EOL)
        optr.reset(new Object(OBJECT_TOKEN, TOKEN_EOL));
    else if (token.value == "EXIT")
        optr.reset(new Object(OBJECT_TOKEN, TOKEN_EXIT));
    else if (token.value == "IF")
        optr.reset(new Object(OBJECT_TOKEN, TOKEN_IF));
    else if (token.value == "THEN")
        optr.reset(new Object(OBJECT_TOKEN, TOKEN_THEN));
    else if (token.value == "ELSE")
        optr.reset(new Object(OBJECT_TOKEN, TOKEN_ELSE));
    else if (token.value == "ENDIF")
        optr.reset(new Object(OBJECT_TOKEN, TOKEN_ENDIF));
    else if (token.value == "FOR")
        optr.reset(new Object(OBJECT_TOKEN, TOKEN_FOR));
    else if (token.value == "ENDFOR")
        optr.reset(new Object(OBJECT_TOKEN, TOKEN_ENDFOR));
    else if (token.value == "WHILE")
        optr.reset(new Object(OBJECT_TOKEN, TOKEN_WHILE));
    else if (token.value == "REPEAT")
        optr.reset(new Object(OBJECT_TOKEN, TOKEN_REPEAT));
    else if (token.value == "ENDWHILE")
        optr.reset(new Object(OBJECT_TOKEN, TOKEN_ENDWHILE));
    else if (token.token == TOKEN_STRING)
    {
        if (token.value == "import")
        {
            std::string filename;
            Token modulename;
            GetToken(src, modulename);
            std::ifstream ifs;
            for (auto& dir : dirs)
            {
                filename = dir + "/" + modulename.value + ".rps";
                ifs.open(filename);
                if (ifs.is_open())
                    break;
            }
            if (!ifs.is_open())
            {
                std::stringstream strm;
                strm << "import: cannot find " << modulename.value << ".rps";
                throw std::runtime_error(strm.str());
            }
            Source srcImport(ifs);
            std::string modname = machine.current_module_;
            machine.current_module_ = modulename.value;
            machine.CreateModule(modulename.value);
            Parse(machine, srcImport);
            machine.current_module_ = modname;
            goto again;
        }
        else
            optr.reset(new String(token.value));
    }

    return true;
}

void Parser::ParseIf(Machine& machine, IfPtr& ifptr, Source& src)
{
    ObjectPtr optr;
    std::vector<ObjectPtr> *pVec = &ifptr->cond;
    while(GetObject(machine, src, optr))
    {
        if (optr->token == TOKEN_ENDIF)
        {
            return;
        }
        else if (optr->token == TOKEN_THEN)
        {
            pVec = &ifptr->then;
            src.prompt = "THEN: ";
        }
        else if (optr->token == TOKEN_ELSE)
        {
            pVec = &ifptr->els;
            src.prompt = "ELSE: ";
        }
        else if (optr->token == TOKEN_IF)
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
        else if (optr->token == TOKEN_FOR)
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
        else if (optr->token == TOKEN_WHILE)
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
        else if (optr->token == TOKEN_START_PROGRAM)
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
        else if (optr->token == TOKEN_START_LIST)
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

void Parser::ParseFor(Machine& machine, ForPtr& forptr, Source& src)
{
    ObjectPtr optr;
    while(GetObject(machine, src, optr))
    {
        if (optr->token == TOKEN_ENDFOR)
        {
            return;
        }
        else if (optr->token == TOKEN_FOR)
        {
            ForPtr forp;
            forp.reset(new For());
            std::string savePrompt = src.prompt;
            src.prompt = "FOR: ";
            ParseFor(machine, forp, src);    // recurse
            src.prompt = savePrompt;
            optr = forp;
        }
        else if (optr->token == TOKEN_IF)
        {
            IfPtr ifp;
            ifp.reset(new If());
            std::string savePrompt = src.prompt;
            src.prompt = "IF: ";
            ParseIf(machine, ifp, src);    // recurse
            src.prompt = savePrompt;
            optr = ifp;
        }
        else if (optr->token == TOKEN_START_PROGRAM)
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
        else if (optr->token == TOKEN_START_LIST)
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

void Parser::ParseWhile(Machine& machine, WhilePtr& whileptr, Source& src)
{
    ObjectPtr optr;
    std::vector<ObjectPtr> *pVec = &whileptr->cond;
    while(GetObject(machine, src, optr))
    {
        if (optr->token == TOKEN_ENDWHILE)
        {
            return;
        }
        else if (optr->token == TOKEN_REPEAT)
        {
            pVec = &whileptr->program;
            src.prompt = "REPEAT: ";
        }
        else if (optr->token == TOKEN_WHILE)
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
        else if (optr->token == TOKEN_FOR)
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
        else if (optr->token == TOKEN_IF)
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
        else if (optr->token == TOKEN_START_PROGRAM)
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
        else if (optr->token == TOKEN_START_LIST)
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
void Parser::ParseList(Machine& machine, ListPtr& lptr, Source& src)
{
    ObjectPtr optr;
    while(GetObject(machine, src, optr))
    {
        if (optr->token == TOKEN_END_LIST)
        {
            return;
        }
        else if (optr->token == TOKEN_START_PROGRAM)
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
        }
        else if (optr->token == TOKEN_START_LIST)
        {
            ListPtr lp;
            lp.reset(new List());
            ParseList(machine, lp, src);    // recurse
            optr = lp;
        }
        lptr->items.push_back(optr);           
    }
}

void Parser::ParseProgram(Machine& machine, ProgramPtr& pptr, Source& src)
{
    ObjectPtr optr;
    pptr->module_name = machine.current_module_;
    while(GetObject(machine, src, optr))
    {
        if (optr->token == TOKEN_END_PROGRAM)
        {
            return;
        }
        else if (optr->token == TOKEN_START_PROGRAM)
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
        else if (optr->token == TOKEN_START_LIST)
        {
            ListPtr lp;
            lp.reset(new List());
            src.prompt = "[] ";
            ParseList(machine, lp, src);    // recurse
            src.prompt = ">> ";
            optr = lp;
        }
        else if (optr->token == TOKEN_IF)
        {
            IfPtr ifp;
            ifp.reset(new If());
            src.prompt = "IF: ";
            ParseIf(machine, ifp, src);    // recurse
            src.prompt = ">> ";
            optr = ifp;
        }
        else if (optr->token == TOKEN_FOR)
        {
            ForPtr forptr;
            forptr.reset(new For());
            std::string savePrompt = src.prompt;
            src.prompt = "FOR: ";
            ParseFor(machine, forptr, src);    // recurse
            src.prompt = savePrompt;
            optr = forptr;
        }
        else if (optr->token == TOKEN_WHILE)
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

void Parser::Parse(Machine& machine, Source& src)
{
    while (!src.istrm.eof())
    {
        ObjectPtr optr;
        while(GetObject(machine, src, optr))
        {
            if (optr->token == TOKEN_EXIT)
            {
                return;
            }
            else if (optr->token == TOKEN_START_LIST)
            {
                src.prompt = "[] ";
                ListPtr lptr;
                lptr.reset(new List());
                ParseList(machine, lptr, src);
                src.prompt = "> ";
                optr = lptr;
            }
            if (optr->token == TOKEN_START_PROGRAM)
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


        // Start of if/else
            if (optr->token == TOKEN_FOR)
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
            if (optr->token == TOKEN_WHILE)
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

            else if (optr->token == TOKEN_IF)
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
            else if (optr->token == TOKEN_EOL)
            {
                if (src.interactive && !optr->bSuppressInteractivePrint)
                {
                    VIEW(machine, 4);
                }
            }
            else
            {
                machine.push(optr);           
            }
        }
    }
}

void Parser::AddCommand(Machine& machine, const char *name, void (*funcptr)(Machine&))
{
    CommandPtr cp;
    cp.reset(new Command(name, funcptr));
    machine.commands.emplace(name, cp);
}

Parser::Parser(Machine& machine)
{
    AddCommand(machine, "SELECT", &SELECT);
    // Stack commands
    AddCommand(machine, "CLRSTK", &CLRSTK);
    AddCommand(machine, "DROP", &DROP);
    AddCommand(machine, "DROPN", &DROPN);
    AddCommand(machine, "SWAP", &SWAP);
    AddCommand(machine, "DUP", &DUP);
    AddCommand(machine, "PICK", &PICK);
    AddCommand(machine, "ROLL", &ROLL);
    AddCommand(machine, "DEPTH", &DEPTH);
    AddCommand(machine, "VIEW", &VIEW);
    AddCommand(machine, "CLONE",  &CLONE);

    // Logical operators
    AddCommand(machine, "EQ", &EQ);
    AddCommand(machine, "NEQ", &NEQ);
    AddCommand(machine, "LT", &LT);
    AddCommand(machine, "LTEQ", &LTEQ);
    AddCommand(machine, "GT", &GT);
    AddCommand(machine, "GTEQ", &GTEQ);
    AddCommand(machine, "AND", &AND);
    AddCommand(machine, "OR", &OR);

    // Variable commands
    AddCommand(machine, "STO", &STO);
    AddCommand(machine, "RCL", &RCL);
    AddCommand(machine, "STOL", &STOL);
    AddCommand(machine, "RCLL", &RCLL);
    AddCommand(machine, "VARNAMES", &VARNAMES);
    AddCommand(machine, "VARTYPES", &VARTYPES);

    // Math commands
    AddCommand(machine, "ADD", &ADD);
    AddCommand(machine, "SUB", &SUB);
    AddCommand(machine, "MUL", &MUL);
    AddCommand(machine, "DIV", &DIV);
    AddCommand(machine, "INC", &INC);
    AddCommand(machine, "DEC", &DEC);

    // Control commands
    AddCommand(machine, "IFT", &IFT);
    AddCommand(machine, "IFTE", &IFTE);
    AddCommand(machine, "TRYCATCH", &TRYCATCH);

    // List commands
    AddCommand(machine, "GET", &GET);
    AddCommand(machine, "APPEND", &APPEND);
    AddCommand(machine, "ERASE", &ERASE);
    AddCommand(machine, "CLEAR", &CLEAR);
    AddCommand(machine, "LIST-INSERT", &LIST_INSERT);
    AddCommand(machine, "MAP-INSERT", &MAP_INSERT);
    AddCommand(machine, "INSERT", &INSERT);
    AddCommand(machine, "SIZE", &SIZE);
    AddCommand(machine, "FIND", &FIND);
    AddCommand(machine, "FIRST", &FIRST);
    AddCommand(machine, "SECOND", &SECOND);
    AddCommand(machine, "TOLIST", &TOLIST);
    AddCommand(machine, "TOMAP", &TOMAP);
    AddCommand(machine, "FROMLIST", &FROMLIST);
    AddCommand(machine, "FROMMAP", &FROMMAP);
    AddCommand(machine, "CREATELIST", &CREATELIST);
    AddCommand(machine, "CREATEMAP", &CREATEMAP);

    // Functional
    AddCommand(machine, "APPLY", &APPLY);
    AddCommand(machine, "SELECT", &SELECT);

    // Execution commands
    AddCommand(machine, "EVAL", &EVAL);
    AddCommand(machine, "CALL", &CALL);

    // Environment
    AddCommand(machine, "MODULES", &MODULES);
    AddCommand(machine, "SETNS", &SETNS);
    AddCommand(machine, "GETNS", &GETNS);
    AddCommand(machine, "CD", &CD);
    AddCommand(machine, "HELP", &HELP);

    // String
    AddCommand(machine, "FORMAT", &FORMAT);
    AddCommand(machine, "CAT", &CAT);
    AddCommand(machine, "JOIN", &JOIN);
    AddCommand(machine, "SUBSTR", &SUBSTR);
    AddCommand(machine, "STRFIND", &STRFIND);
    AddCommand(machine, "STRCMP", &STRCMP);
    AddCommand(machine, "STRNCMP", &STRNCMP);
    AddCommand(machine, "SPLIT", &SPLIT);

    // Types
    AddCommand(machine, "TOINT", &TOINT);
    AddCommand(machine, "TOSTR", &TOSTR);
    AddCommand(machine, "TYPE", &TYPE);

    // IO
    AddCommand(machine, "PRINT", &PRINT);
    AddCommand(machine, "PROMPT", &PROMPT);
    AddCommand(machine, "PREAD", &PREAD);
    AddCommand(machine, "PWRITE", &PWRITE);
    AddCommand(machine, "FWRITE", &FWRITE);
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
            line += ";";
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
            line += ";";
            ++lineno;
            it = line.begin();
        }
    }
}
