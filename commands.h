#pragma once

class Machine;

/*
 * Stack commands
 * */
void DROP(Machine& machine);
void DROPN(Machine& machine);
void SWAP(Machine& machine);
void DUP(Machine& machine);
void PICK(Machine&);
void ROLL(Machine&);
void VIEW(Machine&);

/*
 * Math commands
 */
void ADD(Machine& machine);
void SUB(Machine& machine);
void MUL(Machine& machine);
void DIV(Machine& machine);

/*
 * Variables commands
 */
void STO(Machine& machine);
void RCL(Machine& machine);
void RCL(Machine& machine, const std::string& name, ObjectPtr& out);
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

/* Functional
L1: List
L0: function name

Apply calls the function given in L0 to each item in the list at L1
If the function pushes one value,  a list will be the result in  L0.
If the function pushes multiple values, a list of lists will be in L0.
If the function pushes no values, nothing will be returned in  L0
*/

void APPLY(Machine& machine);

/*
 * Misc commands
 */
void EVAL(Machine&);
void EVAL(Machine&, ObjectPtr);
void CALL(Machine&);
void MODULES(Machine&);




