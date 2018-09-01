#pragma once

class Machine;


StringPtr MakeString();
IntegerPtr MakeInteger();
ProgramPtr MakeProgram();
ListPtr MakeList();
ObjectPtr Clone(ObjectPtr);

bool ToBool(ObjectPtr);

std::string ToStr(Machine&, ObjectPtr);
std::string ToType(Machine&, ObjectPtr);
