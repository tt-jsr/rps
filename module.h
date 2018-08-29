#pragma once

#include <unordered_map>

class Module
{
public:

    std::string module_name_;
    std::unordered_map<std::string, ObjectPtr> variables_;
};
