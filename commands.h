#pragma once

class Machine;

/*
 * Stack commands
 * */
void DROP(Machine& machine);
void DROPN(Machine& machine);
void SWAP(Machine& machine);
void DUP(Machine& machine);
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

/*
 * Control commands
 */

void IFT(Machine& machine);
void IFTE(Machine& machine);

/*
 * List commands
 */
void APPEND(Machine&);
void GET(Machine&);
void INSERT(Machine&);
void ERASE(Machine&);
void CLEAR(Machine&);
void SIZE(Machine&);

/*
 * Misc commands
 */
void EVAL(Machine&);
void CALL(Machine&);




