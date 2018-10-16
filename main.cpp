#include <vector>
#include <unordered_map>
#include <iostream>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include "object.h"
#include "module.h"
#include "machine.h"
#include "parser.h"
#include "utilities.h"

void my_handler(int s)
{
    std::cout << "Interrupt" << std::endl;
    rps::bInterrupt = true;
}

int main(int argc, char *argv[])
{
    rps::Machine machine;
    rps::Parser parser(machine);

   struct sigaction sigIntHandler;

   sigIntHandler.sa_handler = my_handler;
   sigemptyset(&sigIntHandler.sa_mask);
   sigIntHandler.sa_flags = 0;

   sigaction(SIGINT, &sigIntHandler, NULL);

#if defined(CENTOS)
    char *p = getenv("PRE_RUN_PATH");
    if (p)
        setenv("PATH", p, 1);
    unsetenv("LD_LIBRARY_PATH");
    unsetenv("PYTHONPATH");
    unsetenv("PYTHONHOME");
#endif
    machine.current_module_ = "interactive";
    machine.CreateModule("interactive");
    rps::Source src(std::cin);
    src.interactive = true;
    src.prompt = "> ";

    using_history();
    //rps::Import(machine, parser, "init");
    while (true)
    {
        try
        {
            parser.ShellParse(machine, src);
            return 0;
        }
        catch (std::runtime_error& e)
        {
            std::cout << e.what() << std::endl;
        }
    }

    return 0;
}
