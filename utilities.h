#pragma once

StringPtr MakeString();
IntegerPtr MakeInteger();
ProgramPtr MakeProgram();
ListPtr MakeList();
ObjectPtr Clone(ObjectPtr&);

std::string ToStr(ObjectPtr&);
