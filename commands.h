#pragma once

class Machine;

/*
 * Stack commands
 * */

// Drop the TOS
// obj =>
void DROP(Machine& machine);
// Drop n items from the stack
// n => 
void DROPN(Machine& machine);

// Clear the stack
void CLRSTK(Machine& machine);
// Swap L0 and L1
// obj1 obj2 => obj2 obj1
void SWAP(Machine& machine);

// obj => obj obj
void DUP(Machine& machine);

// Pick an item from the nth level of the stack
// and copy to L0
// int => obj
void PICK(Machine&);

// Remove nth item from stack and place at L0
void ROLL(Machine&);
void VIEW(Machine&);
void VIEW(Machine&, size_t n);

// push number of items on the stack
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
void PWRITE(Machine&);
void FWRITE(Machine&);
void SYSTEM(Machine&);

// String commands
void FORMAT(Machine&);
void CAT(Machine&);
void JOIN(Machine&);
void SUBSTR(Machine&);

// Find a string
// "str" startpos "str" => int
// Returns position found, -1 if not found
void STRFIND(Machine&);

// Compare a string. Behaves as strcmp
// "str" "str" => int
// Returns <0 ==0 >0
void STRCMP(Machine&);

// Compare a string. Behaves as strncmp
// "str" "str" length => int
// Returns <0 ==0 >0
void STRNCMP(Machine&);

// Split a string into a list
// "string" "delimiters" opts => [list]
// string: The string to split
// delims: String of character delimiters
// opt: Optional modifiers
//      --collapse: By default, each encountered delimiter will result
//                  in a list item, possibly empty. --collapse willmodify
//                  the result so that only non-empty list items are created
//       --n:       Maximum number of splits to make
void SPLIT(Machine&);

// Types
void TOINT(Machine&);
void TOSTR(Machine&);
void TYPE(Machine&);

// Execution
void EVAL(Machine&);
void EVAL(Machine&, ObjectPtr);
void CALL(Machine&);

// Environment
void MODULES(Machine&);
void SETNS(Machine&);
void GETNS(Machine&);
void CD(Machine&);

/*
 * Misc commands
 */
void CLONE(Machine&);



