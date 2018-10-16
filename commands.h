#pragma once

namespace rps
{

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
void ROLLD(Machine&);
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
void RCLA(Machine& machine);
void VARNAMES(Machine& machine);
void VARTYPES(Machine& machine);
void VARS(Machine& machine);
void REGISTER(Machine&);
void UNREGISTER(Machine&);

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
void SUBLIST(Machine&);
void INSERT(Machine&);
void LINSERT(Machine&);
void ERASE(Machine&);
void CLEAR(Machine&);
void SIZE(Machine&);
void FIRST(Machine&);
void SECOND(Machine&);
void HEAD(Machine&);
void TOLIST(Machine&);
void FROMLIST(Machine&);
void CREATELIST(Machine&);
void UNIQUE(Machine&);
void REVERSE(Machine&);
void ZIP(Machine&);
void UNZIP(Machine&);

// Map
void FIND(Machine&);
void MINSERT(Machine&);
void TOMAP(Machine&);
void FROMMAP(Machine&);
void CREATEMAP(Machine&);
void KEYS(Machine&);
void VALUES(Machine&);

// Functional
void APPLY(Machine& machine);
void APPLY1(Machine& machine);
void SELECT(Machine& machine);
void SELECT1(Machine& machine);
void MAP(Machine& machine);
void MAP1(Machine& machine);
void REDUCE(Machine& machine);

// IO commands
void PRINT(Machine&);
void PROMPT(Machine&);
void PREAD(Machine&);
void PWRITE(Machine&);
void FREAD(Machine&);
void FWRITE(Machine&);
void FSAVE(Machine&);
void FRESTORE(Machine&);
void SYSTEM(Machine&);

// String commands
void FORMAT(Machine&);
void CAT(Machine&);
void JOIN(Machine&);
void SUBSTR(Machine&);
void SUBSTRPOS(Machine&);
void STRFIND(Machine&);
void STRFINDEND(Machine&);
void STRCMP(Machine&);
void STRNCMP(Machine&);
void SPLIT(Machine&);
void STRBEGIN(Machine&);
void STREND(Machine&);
void STRHAS(Machine&);
void STRCSPN(Machine&);

// Types
void TOINT(Machine&);
void TOSTR(Machine&);
void TYPE(Machine&);
void CLONE(Machine&);

// Execution
void EVAL(Machine&);
void EVAL(Machine&, ObjectPtr);
void CALL(Machine&);
void INTERRUPT(Machine&);

// Environment
void NAMESPACES(Machine&);
void SETNS(Machine&);
void GETNS(Machine&);
void CD(Machine&);
void PWD(Machine&);
void HELP(Machine&);


} // namespace rps
