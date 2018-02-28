/******************************
 * Ryan Thompson
 * Project 2
 * CS 3339 - Spring 2018 Section 263
 ******************************/
#include "CPU.h"
#include "Stats.h"
#include <iomanip>

Stats statistics;

const string CPU::regNames[] = {"$zero","$at","$v0","$v1","$a0","$a1","$a2","$a3",
                                "$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7",
                                "$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",
                                "$t8","$t9","$k0","$k1","$gp","$sp","$fp","$ra"};

CPU::CPU(uint32_t pc, Memory &iMem, Memory &dMem) : pc(pc), iMem(iMem), dMem(dMem) {
  for(int i = 0; i < NREGS; i++) {
    regFile[i] = 0;
  }
  hi = 0;
  lo = 0;
  regFile[28] = 0x10008000; // gp
  regFile[29] = 0x10000000 + dMem.getSize(); // sp

  instructions = 0;
  stop = false;
}

void CPU::run() {
  while(!stop) {
    instructions++;

    fetch();
    decode();
    execute();
    mem();
    writeback();

    D(printRegFile());
  }
}

void CPU::fetch() {
  instr = iMem.loadWord(pc);
  pc = pc + 4;
}

/////////////////////////////////////////
// ALL YOUR CHANGES GO IN THIS FUNCTION 
/////////////////////////////////////////
void CPU::decode() {
  uint32_t opcode;      // opcode field
  uint32_t rs, rt, rd;  // register specifiers
  uint32_t shamt;       // shift amount (R-type)
  uint32_t funct;       // funct field (R-type)
  uint32_t uimm;        // unsigned version of immediate (I-type)
  int32_t  simm;        // signed version of immediate (I-type)
  uint32_t addr;        // jump address offset field (J-type)

    opcode = (instr >> 26);     //Register Operation, [31,26] of the instruction
    rs     = (instr >> 21) & 0x1f;  //Source register, [25,21] of the instruction
    rt     = (instr >> 16) & 0x1f;  //Temporary Register, [20,16] of the instruction
    rd     = (instr >> 11) & 0x1f;  //Destination Register [15,11] of the Register Operation
    shamt  = (instr >> 6) & 0x1f;//Shift Amount [10,6] of Register Operation
    funct  = instr & 0x3f;       //Function [5,0] of the Register Operation
    uimm   = instr & 0xffff;      //Unsigned Immediate [15,0]
    simm   = ((int)(instr << 16)) >> 16;      //Signed Immediate [15,0]
    addr   = instr & 0x3FFFFFF;   //address [25,0] of the jump instruction

  // Hint: you probably want to give all the control signals some "safe"
  // default value here, and then override their values as necessary in each
  // case statement below!
    
    
    opIsLoad = false;
    opIsStore = false;
    opIsMultDiv = false;
    writeDest = false; destReg = REG_ZERO;
    aluSrc1 = 0;
    aluSrc2 = 0;
    storeData = 0;
    aluOp = ADD;
     

  D(cout << "  " << hex << setw(8) << pc - 4 << ": ");
  switch(opcode) {
    case 0x00:
      switch(funct) {
        case 0x00: //shfts the data in the rs register with the given amount
                   D(cout << "sll " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt);
                   writeDest = true; destReg = rd;
                   aluOp = SHF_L;
                   aluSrc1 = regFile[rs];
                   aluSrc2 = shamt;
                   break;
        case 0x03: //the same as sll, but to the right
                   D(cout << "sra " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt);
                   writeDest = true; destReg = rd;
                   aluOp = SHF_R;
                   aluSrc1 = regFile[rs];
                   aluSrc2 = shamt;
                   break;
        case 0x08: //the programmer counter is assigned the value in rs
                   D(cout << "jr " << regNames[rs]);
                   pc = regFile[rs];
                   break;
        case 0x10: //moves the hi data into the destination register
                   D(cout << "mfhi " << regNames[rd]);
                   writeDest = true; destReg = rd;
                   aluOp = ADD;
                   aluSrc1 = hi;
                   aluSrc2 = regFile[REG_ZERO];
                   break;
        case 0x12: //moves the lo data into the destination register
                   D(cout << "mflo " << regNames[rd]);
                   writeDest = true; destReg = rd;
                   aluOp = ADD;
                   aluSrc1 = lo;
                   aluSrc2 = regFile[REG_ZERO];
                   break;
        case 0x18: //multiplies the contents in rs and rt and stores them in the hi and lo register
                   D(cout << "mult " << regNames[rs] << ", " << regNames[rt]);
                   opIsMultDiv = true;
                   aluOp = MUL;
                   aluSrc1 = regFile[rs];
                   aluSrc2 = regFile[rt];
                   break;
        case 0x1a: //the same as above but with division
                   D(cout << "div " << regNames[rs] << ", " << regNames[rt]);
                   opIsMultDiv = true;
                   aluOp = DIV;
                   aluSrc1 = regFile[rs];
                   aluSrc2 = regFile[rt];
                   break;
        case 0x21: //adds rs and rt values to be stored as an unsigned value in rd
                   D(cout << "addu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                   writeDest = true; destReg = rd;
                   aluOp = ADD;
                   aluSrc1 = regFile[rs];
                   aluSrc2 = regFile[rt];
                   break;
        case 0x23: //subtracts rt from rs and stores its unsigned value in rd
                   D(cout << "subu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                   writeDest = true; destReg = rd;
                   aluOp = ADD;
                   aluSrc1 = regFile[rs];
                   aluSrc2 = - regFile[rt];
                   break;
        case 0x2a: //stores a 1 in rd if the value of rs is less than rt's value, and a 0 otherwise
                   D(cout << "slt " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                   writeDest = true; destReg = rd;
                   aluOp = CMP_LT;
                   aluSrc1 = regFile[rs];
                   aluSrc2 = regFile[rt];
                   break;
        default: cerr << "unimplemented instruction: pc = 0x" << hex << pc - 4 << endl;
      }
      break;
    case 0x02: //jumps to the branch address
               D(cout << "j " << hex << ((pc & 0xf0000000) | addr << 2)); // P1: pc + 4
               pc = (pc & 0xf0000000) + (addr << 2);
               statistics.flush(2);
               break;
    case 0x03: //jumps to the branch address and sets the program counter
               D(cout << "jal " << hex << ((pc & 0xf0000000) | addr << 2)); // P1: pc + 4
               writeDest = true; destReg = REG_RA; // writes PC+4 to $ra
               aluOp = ADD; // ALU should pass pc thru unchanged
               aluSrc1 = pc;
               aluSrc2 = regFile[REG_ZERO]; // always reads zero
               pc = (pc & 0xf0000000) | addr << 2;
               break;
    case 0x04: //if the values of rs and rt equal, pc is assigned a new address
               D(cout << "beq " << regNames[rs] << ", " << regNames[rt] << ", " << pc + (simm << 2));
               if( regFile[rs] == regFile[rt] )
               {
                   pc = pc + (simm << 2);
                   statistics.countTaken();
               }
               statistics.countBranch();
               break;
    case 0x05: //same results as above if rs' value does not equal rt's
               D(cout << "bne " << regNames[rs] << ", " << regNames[rt] << ", " << pc + (simm << 2));
               if( regFile[rs] != regFile[rt] )
               {
                   pc = pc + (simm << 2);
                   statistics.countTaken();
               }
               statistics.countBranch();
               break;
    case 0x09: //Adds the immidiate unsigned to rs and stores the result in rt
               D(cout << "addiu " << regNames[rt] << ", " << regNames[rs] << ", " << dec << simm);
               aluOp = ADD;
               writeDest = true; destReg = rt;
               aluSrc1 = regFile[rs];
               aluSrc2 = simm;
               break;
    case 0x0c: //compares rs to the zero extended immidiate and stores the result in rt
               D(cout << "andi " << regNames[rt] << ", " << regNames[rs] << ", " << dec << uimm);
               writeDest = true; destReg = rt;
               aluOp = AND;
               aluSrc1 = regFile[rs];
               aluSrc2 = uimm;
               break;
    case 0x0f: //loads the left shifted immidiate in memory
               D(cout << "lui " << regNames[rt] << ", " << dec << simm);
               writeDest = true; destReg = rt;
               aluOp   = ADD;
               aluSrc1 = (uimm << 16);
               aluSrc2 = regFile[REG_ZERO];
               break;
    case 0x1a: //serves as a "software interrupt" in a way
               D(cout << "trap " << hex << addr);
               switch(addr & 0xf) {
                 case 0x0: cout << endl; break;
                 case 0x1: cout << " " << (signed)regFile[rs];
                           break;
                 case 0x5: cout << endl << "? "; cin >> regFile[rt];
                           break;
                 case 0xa: stop = true; break;
                 default: cerr << "unimplemented trap: pc = 0x" << hex << pc - 4 << endl;
                          stop = true;
               }
               break;
    case 0x23: //loads a word created by rs + signed immidiate into memory and rt
               D(cout << "lw " << regNames[rt] << ", " << dec << simm << "(" << regNames[rs] << ")");
               opIsLoad = true;
               aluOp = ADD;
               writeDest = true; destReg = rt;
               aluSrc1 = regFile[rs];
               aluSrc2 = simm;
               break;
    case 0x2b: //This stores the word that load word would normally load into memory and rt
               D(cout << "sw " << regNames[rt] << ", " << dec << simm << "(" << regNames[rs] << ")");
               opIsStore = true;
               aluOp = ADD;
               aluSrc1 = regFile[rs];
               aluSrc2 = simm;
               storeData = regFile[rt];
               aluOut = regFile[rs];
               break;
    default: cerr << "unimplemented instruction: pc = 0x" << hex << pc - 4 << endl;
  }
  D(cout << endl);
}

void CPU::execute() {
  aluOut = alu.op(aluOp, aluSrc1, aluSrc2);
}

void CPU::mem() {
  if(opIsLoad)
  {
    writeData = dMem.loadWord(aluOut);
    statistics.countMemOp();
  }
  else
    writeData = aluOut;

  if(opIsStore)
  {
    dMem.storeWord(storeData, aluOut);
    statistics.countMemOp();
  }
}

void CPU::writeback() {
  if(writeDest && destReg > 0) // skip if write is to reg 0
    regFile[destReg] = writeData;
  
  if(opIsMultDiv) {
    hi = alu.getUpper();
    lo = alu.getLower();
  }
}

void CPU::printRegFile() {
  cout << hex;
  for(int i = 0; i < NREGS; i++) {
    cout << "    " << regNames[i];
    if(i > 0) cout << "  ";
    cout << ": " << setfill('0') << setw(8) << regFile[i];
    if( i == (NREGS - 1) || (i + 1) % 4 == 0 )
      cout << endl;
  }
  cout << "    hi   : " << setfill('0') << setw(8) << hi;
  cout << "    lo   : " << setfill('0') << setw(8) << lo;
  cout << dec << endl;
}

void CPU::printFinalStats() {
  cout << "Program finished at pc = 0x" << hex << pc << "  ("
       << dec << instructions << " instructions executed)" << endl << endl;

    cout << "Cycles: " << statistics.getCycles() << endl;
    cout << "CPI: " << statistics.getCycles()/instructions << endl;
    cout << endl;
    cout << "Bubbles: " << statistics.getBubbles() << endl;
    cout << "Flushes: " << statistics.getFlushes() << endl;
    cout << endl;
    cout << "Mem ops: " << setprecision(1) << fixed << statistics.getMemOps()/(float)instructions*100 << "% of instructions" << endl;
    cout << "Branches: "  << statistics.getBranches() / (float)instructions * 100 << "% of instructions" << endl;
    cout << "  % Taken: " << statistics.getTaken() / (float)statistics.getBranches() * 100 << endl;
}
