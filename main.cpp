#include <vector>
#include <unordered_map>
#include <iostream>
#include "object.h"
#include "module.h"
#include "machine.h"
#include "parser.h"

int main(int argc, char *argv[])
{
    Machine machine;
    Parser parser;

    machine.current_module_ = "interactive";
    machine.CreateModule("interactive");
    Source src(std::cin);
    src.interactive = true;
    src.prompt = "> ";
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
