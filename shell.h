#ifndef _SHELL_H_INCLUDED_
#define _SHELL_H_INCLUDED_

namespace rps
{
    class Machine;

    void PushWord(Machine&, const char *w);
    void PushBar(Machine&);
    void PushLT(Machine&);
    void PushGT(Machine&);
    void PushGTGT(Machine&);
    void PushAmp(Machine&);
    void PushSemi(Machine&);
    void PushNL(Machine&);
}

#endif
