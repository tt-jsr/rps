#pragma once 

namespace rps
{

struct Source
{
    Source(std::istream& is);
    void Read();
    bool iseof();
    std::string line;
    std::string::iterator it;
    std::istream& istrm;
    std::string prompt;
    bool interactive;
    size_t lineno;
};

class RPNParser
{
public:
    RPNParser(Machine&);
    // false at EOL and no data has been read
    bool GetObject(Machine&, Source&, ObjectPtr& optr);
    void Parse(Machine& machine, Source&, std::string& exit);
    void ParseProgram(Machine&, ProgramPtr& pptr, Source& src);
    void ParseList(Machine&, ListPtr& pptr, Source& src);
    void ParseIf(Machine&, IfPtr& ifptr, Source& src);
    void ParseFor(Machine&, ForPtr& forptr, Source& src);
    void ParseWhile(Machine&, WhilePtr& whileptr, Source& src);
    ProgramPtr enclosingProgram;
};

class ShellParser
{
public:
    ShellParser(Machine&);
    // false at EOL and no data has been read
    void Parse(Machine&, Source&, std::string& exit);
    void Parse(Machine&, const std::string&);
};

} // namespace rps

