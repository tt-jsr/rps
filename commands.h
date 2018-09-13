#pragma once

class Machine;

/*
 * Stack commands
 * */
void DROP(Machine& machine);
void DROPN(Machine& machine);
void CLRSTK(Machine& machine);
void SWAP(Machine& machine);
void DUP(Machine& machine);
void PICK(Machine&);
void ROLL(Machine&);
void VIEW(Machine&);
void VIEW(Machine&, size_t n);
void DEPTH(Machine&);

/*
 * Math commands
 */
void ADD(Machine& machine);
void SUB(Machine& machine);
void MUL(Machine& machine);
void DIV(Machine& machine);
void INC(Machine& machine);
void DEC(Machine& machine);

/*
 * Variables commands
 */
void STO(Machine& machine);
void RCL(Machine& machine);
void RCL(Machine& machine, const std::string& name, ObjectPtr& out);
void STOL(Machine& machine);
void RCLL(Machine& machine);
void RCLL(Machine& machine, const std::string& name, ObjectPtr& out);
void VARNAMES(Machine& machine);
void VARTYPES(Machine& machine);

/*
 * Control commands
 */
void IFT(Machine& machine);
void IFTE(Machine& machine);
void TRYCATCH(Machine& machine);

/*
 * Logical Operators
 */
void EQ(Machine&);
void NEQ(Machine&);
void LT(Machine&);
void LTEQ(Machine&);
void GT(Machine&);
void GTEQ(Machine&);
void NOT(Machine&);
void AND(Machine&);
void OR(Machine&);

/*
 * List commands
 */
void APPEND(Machine&);
void GET(Machine&);
void FIND(Machine&);
void INSERT(Machine&);
void LIST_INSERT(Machine&);
void MAP_INSERT(Machine&);
void ERASE(Machine&);
void CLEAR(Machine&);
void SIZE(Machine&);
void FIRST(Machine&);
void SECOND(Machine&);
void TOLIST(Machine&);
void TOMAP(Machine&);
void FROMLIST(Machine&);
void FROMMAP(Machine&);
void CREATELIST(Machine&);
void CREATEMAP(Machine&);

// Functional
void APPLY(Machine& machine);
void SELECT(Machine& machine);

// IO commands
void PRINT(Machine&);
void PROMPT(Machine&);
void PREAD(Machine&);
void SYSTEM(Machine&);

// String commands
void FORMAT(Machine&);
void CAT(Machine&);
void JOIN(Machine&);
void SUBSTR(Machine&);
void STRFIND(Machine&);
void STRCMP(Machine&);
void SPLIT(Machine&);

// Types
void TOINT(Machine&);
void TOSTR(Machine&);
void TYPE(Machine&);

/*
 * Misc commands
 */
void EVAL(Machine&);
void EVAL(Machine&, ObjectPtr);
void CALL(Machine&);
void MODULES(Machine&);
void CLONE(Machine&);




