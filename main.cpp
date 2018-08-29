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
    parser.Parse(machine, src);
    return 0;
}
