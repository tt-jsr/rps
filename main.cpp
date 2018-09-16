#include <vector>
#include <unordered_map>
#include <iostream>
#include <readline/readline.h>
#include <readline/history.h>
#include "object.h"
#include "module.h"
#include "machine.h"
#include "parser.h"

int main(int argc, char *argv[])
{
    Machine machine;
    Parser parser(machine);

    machine.current_module_ = "interactive";
    machine.CreateModule("interactive");
    Source src(std::cin);
    src.interactive = true;
    src.prompt = "> ";

    using_history();
    while (true)
    {
        try
        {
            parser.Parse(machine, src);
            return 0;
        }
        catch (std::runtime_error& e)
        {
            std::cout << e.what() << std::endl;
        }
    }

    return 0;
}
