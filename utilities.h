#pragma once

StringPtr MakeString();
IntegerPtr MakeInteger();
ProgramPtr MakeProgram();
ListPtr MakeList();
ObjectPtr Clone(ObjectPtr);

bool ToBool(ObjectPtr);

std::string ToStr(ObjectPtr&);
