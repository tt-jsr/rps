#pragma once

StringPtr MakeString();
IntegerPtr MakeInteger();
ProgramPtr MakeProgram();
ObjectPtr Clone(ObjectPtr&);

std::string ToStr(ObjectPtr&);
