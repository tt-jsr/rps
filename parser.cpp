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

void CollectQuoted(Source& src, std::string& out)
{
    assert(*src.it == '\"');
    ++src.it;
    while(!src.istrm.eof())
    {
        while (src.it != src.line.end())
        {
            if (*src.it == '\"')
            {
                ++src.it;
                return;
            }
            out.push_back(*src.it);
            ++src.it;
        }
        src.Read();
    }
}

void Collect(Source& src, std::string& out, bool& quoted)
{
    quoted = false;
    out.clear();
    SkipWhitespace(src);
    while (src.it != src.line.end())
    {
        if (*src.it == '\"')
        {
            CollectQuoted(src, out);
            quoted = true;
            return;
        }
        if (*src.it == ' ' || *src.it == '\t')
            return;
        out.push_back(*src.it);
        ++src.it;
    }
}

bool Parser::GetObject(Machine& machine, Source& src, ObjectPtr& optr)
{
again:
    std::string w;
    bool quoted;
    Collect(src, w, quoted);
    if (w.empty() && (src.it == src.line.end()))
        return false;
    if (quoted)
    {
        optr.reset(new String(w));
        return true;
    }
    if (w == "import")
    {
        std::string filename;
        std::string modulename;
        Collect(src, modulename, quoted);
        std::ifstream ifs;
        filename = modulename + ".rps";
        ifs.open(filename);
        if (!ifs.is_open())
        {
            std::stringstream strm;
            strm << "import: cannot open " << filename;
            throw std::runtime_error(strm.str());
        }
        Source srcImport(ifs);
        std::string modname = machine.current_module_;
        machine.current_module_ = modulename;
        machine.CreateModule(modulename);
        Parse(machine, srcImport);
        machine.current_module_ = modname;
        goto again;
    }
    // Stack commands
    if (w == "STO")
        optr.reset(new Command(w, &STO));
    else if (w == "RCL")
        optr.reset(new Command(w, &RCL));
    else if (w == "DROP")
        optr.reset(new Command(w, &DROP));
    else if (w == "DROPN")
        optr.reset(new Command(w, &DROPN));
    else if (w == "SWAP")
        optr.reset(new Command(w, &SWAP));
    else if (w == "DUP")
        optr.reset(new Command(w, &DUP));
    else if (w == "PICK")
        optr.reset(new Command(w, &PICK));
    else if (w == "VIEW")
    {
        optr.reset(new Command(w, &VIEW));
        optr->bSuppressInteractivePrint = true;
    }

    // Logical operators
    else if (w == "EQ")
        optr.reset(new Command(w, &EQ));
    else if (w == "NEQ")
        optr.reset(new Command(w, &NEQ));
    else if (w == "LT")
        optr.reset(new Command(w, &LT));
    else if (w == "LTEQ")
        optr.reset(new Command(w, &LTEQ));
    else if (w == "GT")
        optr.reset(new Command(w, &GT));
    else if (w == "GTEQ")
        optr.reset(new Command(w, &GTEQ));
    else if (w == "AND")
        optr.reset(new Command(w, &AND));
    else if (w == "OR")
        optr.reset(new Command(w, &OR));

    // Math commands
    else if (w == "ADD")
        optr.reset(new Command(w, &ADD));
    else if (w == "SUB")
        optr.reset(new Command(w, &SUB));
    else if (w == "MUL")
        optr.reset(new Command(w, &MUL));
    else if (w == "DIV")
        optr.reset(new Command(w, &DIV));

    // Control commands
    else if (w == "IFT")
        optr.reset(new Command(w, &IFT));
    else if (w == "IFTE")
        optr.reset(new Command(w, &IFTE));

    // List commands
    else if (w == "GET")
        optr.reset(new Command(w, &GET));
    else if (w == "APPEND")
        optr.reset(new Command(w, &APPEND));
    else if (w == "ERASE")
        optr.reset(new Command(w, &ERASE));
    else if (w == "CLEAR")
        optr.reset(new Command(w, &CLEAR));
    else if (w == "INSERT")
        optr.reset(new Command(w, &INSERT));
    else if (w == "SIZE")
        optr.reset(new Command(w, &SIZE));

    // Execution commands
    else if (w == "EVAL")
        optr.reset(new Command(w, &EVAL));
    else if (w == "CALL")
        optr.reset(new Command(w, &CALL));

    // tokens
    else if (w == "<<")
        optr.reset(new Object(OBJECT_TOKEN, TOKEN_START_PROGRAM));
    else if (w == ">>")
        optr.reset(new Object(OBJECT_TOKEN, TOKEN_END_PROGRAM));
    else if (w == "[")
        optr.reset(new Object(OBJECT_TOKEN, TOKEN_START_LIST));
    else if (w == "]")
        optr.reset(new Object(OBJECT_TOKEN, TOKEN_END_LIST));
    else if ((w[0] >= '0' && w[0] <= '9') || w[0] == '-' )
        optr.reset(new Integer(strtol(w.c_str(), nullptr, 10)));
    else
        optr.reset(new String(w));

    return true;
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

            if (src.interactive && optr->token == TOKEN_COMMAND && !optr->bSuppressInteractivePrint)
            {
                machine.push(optr);           
                if (!machine.stack_.empty())
                {
                    ObjectPtr& optr = machine.peek();
                    std::string s = ToStr(optr);
                    std::cout << s << std::endl;
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
        ++lineno;
        it = line.begin();
    }
}
