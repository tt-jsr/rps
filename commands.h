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
 * Misc commands
 */
void EVAL(Machine&);




