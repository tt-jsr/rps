#pragma once 

struct Source
{
    Source(std::istream& is);
    void Read();
    std::string line;
    std::string::iterator it;
    std::istream& istrm;
    std::string prompt;
    bool interactive;
    size_t lineno;
};

class Parser
{
public:
    // false at EOL and no data has been read
    bool GetObject(Machine&, Source&, ObjectPtr& optr);
    void Parse(Machine& machine, Source&);
    void ParseProgram(Machine&, ProgramPtr& pptr, Source& src);
    void ParseList(Machine&, ListPtr& pptr, Source& src);
};
