#include <vector>
#include <unordered_map>
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
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
again:
    Token token;
    GetToken(src, token);
    if (token.value.empty() && (src.it == src.line.end()))
        return false;
    //std::cout << "===GetObject: " << token.token << ":\"" << token.value << "\"" << std::endl;

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
            //std::cout << "=== importing " << std::endl;
            std::string filename;
            Token modulename;
            GetToken(src, modulename);
            std::ifstream ifs;
            filename = modulename.value + ".rps";
            ifs.open(filename);
            if (!ifs.is_open())
            {
                std::stringstream strm;
                strm << "import: cannot open " << filename;
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
        // Stack commands
        else if (token.value == "CLRSTK")
            optr.reset(new Command(token.value, &CLRSTK));
        else if (token.value == "DROP")
            optr.reset(new Command(token.value, &DROP));
        else if (token.value == "DROPN")
            optr.reset(new Command(token.value, &DROPN));
        else if (token.value == "SWAP")
            optr.reset(new Command(token.value, &SWAP));
        else if (token.value == "DUP")
            optr.reset(new Command(token.value, &DUP));
        else if (token.value == "PICK")
            optr.reset(new Command(token.value, &PICK));
        else if (token.value == "ROLL")
            optr.reset(new Command(token.value, &ROLL));
        else if (token.value == "DEPTH")
            optr.reset(new Command(token.value, &DEPTH));
        else if (token.value == "VIEW")
        {
            optr.reset(new Command(token.value, &VIEW));
            optr->bSuppressInteractivePrint = true;
        }
        else if (token.value == "CLONE")
            optr.reset(new Command (token.value, &CLONE));

        // Logical operators
        else if (token.value == "EQ")
            optr.reset(new Command(token.value, &EQ));
        else if (token.value == "NEQ")
            optr.reset(new Command(token.value, &NEQ));
        else if (token.value == "LT")
            optr.reset(new Command(token.value, &LT));
        else if (token.value == "LTEQ")
            optr.reset(new Command(token.value, &LTEQ));
        else if (token.value == "GT")
            optr.reset(new Command(token.value, &GT));
        else if (token.value == "GTEQ")
            optr.reset(new Command(token.value, &GTEQ));
        else if (token.value == "AND")
            optr.reset(new Command(token.value, &AND));
        else if (token.value == "OR")
            optr.reset(new Command(token.value, &OR));

        // Variable commands
        else if (token.value == "STO")
            optr.reset(new Command(token.value, &STO));
        else if (token.value == "RCL")
            optr.reset(new Command(token.value, &RCL));
        else if (token.value == "STOL")
            optr.reset(new Command(token.value, &STOL));
        else if (token.value == "RCLL")
            optr.reset(new Command(token.value, &RCLL));
        else if (token.value == "VARNAMES")
            optr.reset(new Command(token.value, &VARNAMES));
        else if (token.value == "VARTYPES")
            optr.reset(new Command(token.value, &VARTYPES));

        // Math commands
        else if (token.value == "ADD")
            optr.reset(new Command(token.value, &ADD));
        else if (token.value == "SUB")
            optr.reset(new Command(token.value, &SUB));
        else if (token.value == "MUL")
            optr.reset(new Command(token.value, &MUL));
        else if (token.value == "DIV")
            optr.reset(new Command(token.value, &DIV));
        else if (token.value == "INC")
            optr.reset(new Command(token.value, &INC));
        else if (token.value == "DEC")
            optr.reset(new Command(token.value, &DEC));

        // Control commands
        else if (token.value == "IFT")
            optr.reset(new Command(token.value, &IFT));
        else if (token.value == "IFTE")
            optr.reset(new Command(token.value, &IFTE));
        else if (token.value == "TRYCATCH")
            optr.reset(new Command(token.value, &TRYCATCH));

        // List commands
        else if (token.value == "GET")
            optr.reset(new Command(token.value, &GET));
        else if (token.value == "APPEND")
            optr.reset(new Command(token.value, &APPEND));
        else if (token.value == "ERASE")
            optr.reset(new Command(token.value, &ERASE));
        else if (token.value == "CLEAR")
            optr.reset(new Command(token.value, &CLEAR));
        else if (token.value == "LIST-INSERT")
            optr.reset(new Command(token.value, &LIST_INSERT));
        else if (token.value == "MAP-INSERT")
            optr.reset(new Command(token.value, &MAP_INSERT));
        else if (token.value == "INSERT")
            optr.reset(new Command(token.value, &INSERT));
        else if (token.value == "SIZE")
            optr.reset(new Command(token.value, &SIZE));
        else if (token.value == "FIND")
            optr.reset(new Command(token.value, &FIND));
        else if (token.value == "FIRST")
            optr.reset(new Command(token.value, &FIRST));
        else if (token.value == "SECOND")
            optr.reset(new Command(token.value, &SECOND));
        else if (token.value == "TOLIST")
            optr.reset(new Command(token.value, &TOLIST));
        else if (token.value == "TOMAP")
            optr.reset(new Command(token.value, &TOMAP));
        else if (token.value == "FROMLIST")
            optr.reset(new Command(token.value, &FROMLIST));
        else if (token.value == "FROMMAP")
            optr.reset(new Command(token.value, &FROMMAP));
        else if (token.value == "CREATELIST")
            optr.reset(new Command(token.value, &CREATELIST));
        else if (token.value == "CREATEMAP")
            optr.reset(new Command(token.value, &CREATEMAP));

        // Functional
        else if (token.value == "APPLY")
            optr.reset(new Command(token.value, &APPLY));
        else if (token.value == "SELECT")
            optr.reset(new Command(token.value, &SELECT));

        // Execution commands
        else if (token.value == "EVAL")
            optr.reset(new Command(token.value, &EVAL));
        else if (token.value == "CALL")
            optr.reset(new Command(token.value, &CALL));

        // Environment
        else if (token.value == "MODULES")
            optr.reset(new Command(token.value, &MODULES));

        // String
        else if (token.value == "FORMAT")
            optr.reset(new Command(token.value, &FORMAT));
        else if (token.value == "CAT")
            optr.reset(new Command(token.value, &CAT));
        else if (token.value == "JOIN")
            optr.reset(new Command(token.value, &JOIN));
        else if (token.value == "SUBSTR")
            optr.reset(new Command(token.value, &SUBSTR));
        else if (token.value == "STRFIND")
            optr.reset(new Command(token.value, &STRFIND));
        else if (token.value == "STRCMP")
            optr.reset(new Command(token.value, &STRCMP));
        else if (token.value == "SPLIT")
            optr.reset(new Command(token.value, &SPLIT));

        // Types
        else if (token.value == "TOINT")
            optr.reset(new Command(token.value, &TOINT));
        else if (token.value == "TOSTR")
            optr.reset(new Command(token.value, &TOSTR));
        else if (token.value == "TYPE")
            optr.reset(new Command(token.value, &TYPE));

        // IO
        else if (token.value == "PRINT")
            optr.reset(new Command(token.value, &PRINT));
        else if (token.value == "PROMPT")
            optr.reset(new Command(token.value, &PROMPT));
        else if (token.value == "PREAD")
            optr.reset(new Command(token.value, &PREAD));
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
            ParseProgram(machine, pptr, src);  
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
            ParseProgram(machine, pptr, src);    // recurse
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
            ParseProgram(machine, pptr, src);    // recurse
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
            ParseProgram(machine, pptr, src);    // recurse
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
            ParseProgram(machine, pp, src);    // recurse
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
            if (optr->token == TOKEN_START_LIST)
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
                ParseProgram(machine, pptr, src);
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

/********************************************************/
Source::Source(std::istream& is)
:istrm(is)
, it(line.end())
, interactive(false)
, lineno(0)
{
}

void Source::Read()
{
    if (istrm.eof())
    {
        it = line.end();
        return;
    }
    if (it == line.end())
    {
        if (interactive)
            std::cout << prompt << std::flush;
        getline(istrm, line);
        line += ";";
        ++lineno;
        it = line.begin();
    }
}
