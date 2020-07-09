#include <iostream>

namespace sjtu{

enum InstructionOperation {
    BUBBLE, LUI, AUIPC, JAL, JALR, BEQ, BNE, BLT, BGE, BLTU, BGEU, LB, LH, LW, LBU, LHU, SB, SH, SW, ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI, ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND
};
enum InstructionOpcode {
    OC_BUBBLE  = 0b0000000,
    OC_LUI     = 0b0110111,
    OC_AUIPC   = 0b0010111,
    OC_JAL     = 0b1101111,
    OC_JALR    = 0b1100111,
    OC_BRANCH  = 0b1100011,
    OC_LOAD    = 0b0000011,
    OC_STORE   = 0b0100011,
    OC_OP_IMM  = 0b0010011,
    OC_OP      = 0b0110011
};

class TwoBitSaturatingCounter {
private:
    uint8_t curState;
public:
    bool get() {
        return curState >> 1 & 1;
    }
    void set(bool PredictResult) {
        PredictResult ? ++curState : --curState;
        curState = curState ^ (curState & 0b100) ^ (curState & 0b100 >> 1) ^ (curState & 0b100 >> 2);
    }
};

typedef TwoBitSaturatingCounter BranchPredictor;

class ArithmeticLogicUnit {
private:
    uint input1, input2; 
public:
    enum OpType {
        OT_NOP, OT_ADD, OT_SUB, OT_SLL, OT_SLT, OT_SLTU, OT_XOR, OT_SRA, OT_SRL, OT_OR, OT_AND
    } opType;
    void setOpType(OpType type) { opType = type; }
    void setOpType(const uint &funct3, const uint &funct7) {
        switch (funct3) {
        case 0b000: // ADD SUB
            if (funct7 == 0)
                this->opType = OT_ADD;
            else 
                this->opType = OT_SUB;
            break;
        case 0b001: // SLL
            this->opType = OT_SLL; break;
        case 0b010: // SLT
            this->opType = OT_SLT; break;
        case 0b011: // SLTU
            this->opType = OT_SLTU; break;
        case 0b100: // XOR
            this->opType = OT_XOR; break;
        case 0b101: // SRA SRL
            if (funct7 == 0)
                this->opType = OT_SRA; 
            else
                this->opType = OT_SRL; 
            break;
        case 0b110: // OR
            this->opType = OT_OR; break;
        case 0b111: // AND
            this->opType = OT_AND; break;
        default:
            this->opType = OT_NOP;  break;
        }
    }
    void setInput1(const uint &value) {
        input1 = value;
    }
    void setInput2(const uint &value) {
        input2 = value;
    }
    uint getOutput() {
        switch (opType) {
            case OT_NOP: return 0u; 
            case OT_ADD: return input1 + input2;
            case OT_SUB: return input1 - input2;
            case OT_SLL: return input1 << input2;
            case OT_SLT: return (int)input1 < (int)input2 ? 1 : 0;
            case OT_SLTU: return input1 < input2 ? 1 : 0;
            case OT_XOR: return input1 ^ input2;
            case OT_SRA: return (int)input1 >> input2;
            case OT_SRL: return input1 >> input2;
            case OT_OR: return input1 | input2;
            case OT_AND: return input1 & input2;
        }
        return 0u;
    }
};

class AdderUnit {
private:
    uint input1, input2;
public:
    void setInput1(uint value) { input1 = value; }
    void setInput2(uint value) { input2 = value; }
    uint getOutput() { return input1 + input2; }
};

class RandomAccessMemory {
private:
    uint8_t * Mem;
    uint32_t size;
public:
    RandomAccessMemory(uint32_t size = 1 << 20) : size(size) {
        Mem = new uint8_t[size];
    }
    uint8_t read(const uint32_t &pointer) { 
        return Mem[pointer]; 
    }
    void write(const uint32_t &pointer, uint8_t value) { 
        Mem[pointer] = value; 
    }
    ~RandomAccessMemory() { delete [] Mem; }
};

class InstructionDecoder {
private:
    uint32_t IR;
    uint32_t substr_of_inst(uint32_t low, uint32_t len) {
        return IR >> low & ((1u << len) - 1u);
    }
    uint32_t substr_of_inst(uint32_t pos) {
        return IR >> pos & 1u;
    }
    uint32_t sign_extend(uint32_t low) {
        uint32_t rst = 0u, sign = substr_of_inst(31) << low;
        while (low < 32u) 
            rst |= sign, low++, sign <<= 1;
        return rst;
    }
    enum InstructionFormat {
        R_type, I_type, S_type, B_type, U_type, J_type
    };
    InstructionFormat getFormat() {
        uint opcode = getOpcode();
        switch (opcode) {
            case 0b0110111: case 0b0010111: 
                return U_type;
            case 0b1101111:
                return J_type;
            case 0b1100111:
                return I_type;
            case 0b1100011:
                return B_type;
            case 0b0000111:
                return I_type;
            case 0b0100011:
                return S_type;
            case 0b0010011:
                uint funct3 = substr_of_inst(12, 3);
                if (funct3 != 0b001 and funct3 != 0b101)
                    return I_type;
                else 
                    return R_type;
            case 0b0110011:
                return R_type;
        }
    }
public:
    InstructionDecoder(const uint32_t IR) : IR(IR) {}
    InstructionOpcode getOpcode() { 
        return InstructionOpcode(substr_of_inst(0, 7)); 
    }
    bool checkRd() {
        InstructionOpcode opcode = getOpcode();
        return opcode != OC_BRANCH and opcode != OC_STORE;
    }
    bool checkRs1() {
        InstructionOpcode opcode = getOpcode();
        return opcode != OC_LUI and opcode != OC_AUIPC and opcode != OC_JAL;
    }
    bool checkRs2() {
        InstructionOpcode opcode = getOpcode();
        return checkRs1() and opcode != OC_LOAD and opcode != OC_OP_IMM;
    }
    bool checkShamt() {
        return getOpcode() == OC_OP_IMM and (getFunct3() == 0b001 or getFunct3() == 0b101);
    }
    uint32_t getRd()        { return substr_of_inst(7, 5); }
    uint32_t getFunct3()    { return substr_of_inst(12, 3); }
    uint32_t getRs1()       { return substr_of_inst(15, 5); }
    uint32_t getRs2()       { return substr_of_inst(20, 5); }
    uint32_t getShamt()     { return substr_of_inst(20, 5); }
    uint32_t getFunct7()    { return substr_of_inst(25, 7); }
    uint32_t getImm() {
        switch (getFormat()) {
            case R_type: return 0u;
            case I_type: 
                return (substr_of_inst(20, 12) | sign_extend(11)); 
            case S_type: 
                return (substr_of_inst(25, 7) << 5 | substr_of_inst(7, 5) | sign_extend(11));
            case B_type: 
                return (substr_of_inst(31) << 12 | substr_of_inst(25, 6) << 5 | substr_of_inst(8, 4) << 1 | substr_of_inst(7) | sign_extend(12));
            case U_type:
                return (substr_of_inst(12, 20) << 12 | sign_extend(31));
            case J_type:
                return (substr_of_inst(31) << 20 | substr_of_inst(21, 10) << 1 | substr_of_inst(20) << 11 | substr_of_inst(12, 8) << 12 | sign_extend(20));
        }
    }
    InstructionOperation getOperation() {
        uint opcode = getOpcode();
        uint funct3 = getFunct3();
        uint funct7 = getFunct7();

        switch (opcode) {
            default: return BUBBLE;
            case 0b0110111: return LUI;
            case 0b0010111: return AUIPC;
            case 0b1101111: return JAL;
            case 0b1100111: return JALR;
            case 0b1100011: 
                switch (funct3) {
                    case 0b000: return BEQ; //BEQ
                    case 0b001: return BNE; //BNE
                    case 0b100: return BLT; //BLT
                    case 0b101: return BGE; //BGE
                    case 0b110: return BLTU; //BLTU
                    case 0b111: return BGEU; //BGEU
                    default: return BUBBLE;
                }
                break;
            case 0b0000011:
                switch (funct3) {
                    case 0b000: return LB;  //LB
                    case 0b001: return LH;  //LH
                    case 0b010: return LW;  //LW
                    case 0b100: return LBU; //LBU
                    case 0b101: return LHU; //LHU
                    default: return BUBBLE;
                }
            case 0b0100011:
                switch (funct3) {
                    case 0b000: return SB; //SB
                    case 0b001: return SH; //SH
                    case 0b010: return SW; //SW
                    default: return BUBBLE;
                }
            case 0b0010011:
                if (funct3 != 0b001 and funct3 != 0b101) {
                    switch (funct3) {
                        case 0b000: return ADDI;    //ADDI
                        case 0b010: return SLTI;    //SLTI
                        case 0b011: return SLTIU;   //SLTIU
                        case 0b100: return XORI;    //XORI
                        case 0b110: return ORI;     //ORI
                        case 0b111: return ANDI;    //ANDI
                        default: return BUBBLE;
                    }
                } else {
                    if (funct3 == 0b001)
                        return SLLI; //SLLI
                    else if (funct7 == 0) 
                        return SRLI; //SRLI
                    else 
                        return SRAI; //SRAI
                }
                break;
            case 0b0110011:
                switch (funct3) {
                    case 0b000: return ADDI;   
                        if (funct7 == 0)
                            return ADD; // ADD
                        else 
                            return SUB; //SUB
                        break;         
                    case 0b001: return SLL;     //SLL
                    case 0b010: return SLT;     //SLT
                    case 0b011: return SLTU;    //SLTU
                    case 0b100: return XOR;     //XOR
                    case 0b101: 
                        if (funct7 == 0)
                            return SRL; //SRL
                        else
                            return SRA; //SRA
                        break; 
                    case 0b110: return OR;      //OR
                    case 0b111: return AND;     //AND
                    default: return BUBBLE;
                }
        }
    }  
};

class Registers {
private:
    uint32_t Regs[32];
public:
    uint32_t read(uint32_t pointer) {
        return Regs[pointer];
    }
    void write(uint32_t pointer, uint32_t value) {
        Regs[pointer] = value;
    }
};

class Forwarding {
private:
    uint32_t ex_rd;
    uint32_t ex_output;
    uint32_t mem_rd;
    uint32_t mem_output;
public:
    Forwarding() {
        clear();
    }
    void clear() {
        update_ex();
        update_mem();
    }
    void update_ex(uint32_t rd = 0, uint32_t output = 0) {
        ex_rd =  rd;
        ex_output = output;
    }
    void update_mem(uint32_t rd = 0, uint32_t output = 0) {
        mem_rd = rd;
        mem_output = output;
    }
    bool check_changed(const uint32_t &pointer) {
        if (pointer != 0 and (pointer == ex_rd or pointer == mem_rd))
            return true;
    }
    uint32_t get(const uint32_t &pointer) {
        if (pointer == ex_rd)
            return ex_output;
        else
            return mem_output;
    }
};
struct IF_ID { 
    uint32_t IR; 
    uint32_t NPC; 
    uint32_t PC; 
    void clear() { IR = 0; }
};
struct ID_EX { 
    uint32_t IR; 
    uint32_t NPC;
    uint32_t PC; 
    uint32_t A; 
    uint32_t B;
    void clear() { IR = 0; }
};
struct EX_MEM { 
    uint32_t IR; 
    uint32_t ALUOutput; 
    uint32_t B; 
    bool cond; 
    void clear() { IR = 0; }
};
struct MEM_WB { 
    uint32_t IR; 
    uint32_t ALUOutput; 
    uint32_t LMD; 
    void clear() { IR = 0; }
};

class Hazard {
private:
    bool exist;
public:
    Hazard() { exist = false; }
    void set(bool state) { exist = state; }
    bool get() { return exist; }
};


class StageUnit {
private:
    uint64_t remain_clk_cyc;
    Hazard hazard;
public:
    StageUnit() {}
    void set_clk_cyc(uint64_t t) {
        remain_clk_cyc = t;
    } 
    void start() {
        set_clk_cyc(1);
    }
    void run() {
        if (remain_clk_cyc > 0)
            remain_clk_cyc--;
    }
    bool is_finished() {
        return remain_clk_cyc == 0;
    }
    void throw_hazard() {
        hazard.set(true);
    }
    bool catch_hazard() {
        return hazard.get();
    }
    void solve_hazard() {
        hazard.set(false);
    }
};

class IFU : public StageUnit {
private:
    uint32_t &pc;
    IF_ID &if_id;
    RandomAccessMemory &RAM;
    BranchPredictor &BP;
    AdderUnit AU;
public:
    IFU(uint32_t &pc, IF_ID &if_id, RandomAccessMemory &RAM, BranchPredictor &BP) : pc(pc), if_id(if_id), RAM(RAM), BP(BP) {}
    void start() {
        StageUnit::start();
        if_id.PC = pc;
        if_id.IR = 
            RAM.read(pc) | 
            RAM.read(pc + 1) << 8 | 
            RAM.read(pc + 2) << 16 | 
            RAM.read(pc + 3) << 24;
       
        InstructionDecoder IR(if_id.IR);

        AU.setInput1(pc);
        switch (IR.getOpcode()) {
            case OC_JAL:    AU.setInput2(IR.getImm());
                break;
            case OC_JALR:   AU.setInput2(4);
                break;
            case OC_BRANCH: BP.get() ? 
                            AU.setInput2(IR.getImm()) : 
                            AU.setInput2(4); 
                break;
            default:        AU.setInput2(4); 
                break;
        }
        pc = AU.getOutput();
        if_id.NPC = pc;
    }
};



class IDU : public StageUnit {
private:
    uint32_t &pc;
    IF_ID &if_id;
    ID_EX &id_ex;
    Registers &Regs;
    BranchPredictor &BP;
    Forwarding &FD;
    AdderUnit AU;
public:
    IDU(uint32_t &pc, IF_ID &if_id, ID_EX &id_ex, Registers &Regs, BranchPredictor &BP, Forwarding &FD) : 
    pc(pc), if_id(if_id), id_ex(id_ex), Regs(Regs), BP(BP), FD(FD) {}
    void start() {
        StageUnit::start();
        if (if_id.IR == BUBBLE) return;
        id_ex.IR = if_id.IR;

        InstructionDecoder IR(if_id.IR);
        if (IR.checkRs1()) {
            if (FD.check_changed(IR.getRs1()))
                id_ex.A = FD.get(IR.getRs1());
            else 
                id_ex.A = Regs.read(IR.getRs1());
        }
            
        if (IR.checkRs2()) {
            if (FD.check_changed(IR.getRs2()))
                id_ex.B = FD.get(IR.getRs2());
            else 
                id_ex.B = Regs.read(IR.getRs2());
        } else if (IR.checkShamt())
            id_ex.B = IR.getShamt();
        else
            id_ex.B = IR.getImm();

        if (IR.getOpcode() == OC_JALR) {
            AU.setInput1(if_id.PC);
            AU.setInput2(IR.getImm());
            pc = AU.getOutput() & ~1u;
        }

        if_id.clear();
    }
};

class EXU : public StageUnit {
private:
    uint32_t &pc;
    ID_EX &id_ex;
    EX_MEM &ex_mem;
    BranchPredictor &BP;
    Forwarding &FD;
    ArithmeticLogicUnit ALU;
public:
    EXU(
        uint32_t &pc, 
        ID_EX &id_ex, 
        EX_MEM &ex_mem, 
        BranchPredictor &BP, 
        Forwarding &FD
    ) : pc(pc), id_ex(id_ex), ex_mem(ex_mem), BP(BP), FD(FD) {}
    void start() {
        StageUnit::start();
        if (id_ex.IR == BUBBLE) return;
        ex_mem.IR = id_ex.IR;
        InstructionDecoder IR(id_ex.IR);
        
        switch (IR.getOpcode())
        {
        case OC_OP_IMM: case OC_OP: 
            ALU.setInput1(id_ex.A);
            ALU.setInput2(id_ex.B);
            ALU.setOpType(IR.getFunct3(), IR.getFunct7());
            ex_mem.ALUOutput = ALU.getOutput(); 
            break;
        case OC_LUI: 
            ALU.setInput1(id_ex.B);
            ALU.setInput2(12);
            ALU.setOpType(ArithmeticLogicUnit::OT_SRL);
            ex_mem.ALUOutput = ALU.getOutput(); 
            break;
        case OC_AUIPC: 
            ALU.setInput1(id_ex.B);
            ALU.setInput2(12);
            ALU.setOpType(ArithmeticLogicUnit::OT_SRL);
            ALU.setInput1(ALU.getOutput());
            ALU.setInput2(id_ex.PC);
            ALU.setOpType(ArithmeticLogicUnit::OT_ADD);
            ex_mem.ALUOutput = ALU.getOutput();
            break;
        case OC_JAL: case OC_JALR:
            ALU.setInput1(id_ex.PC);
            ALU.setInput2(4);
            ALU.setOpType(ArithmeticLogicUnit::OT_ADD);
            ex_mem.ALUOutput = ALU.getOutput();
            break;
        case OC_BRANCH:
            bool token;
            uint32_t operation = IR.getOperation();
            switch (operation) {
                case BEQ: case BNE:
                    ALU.setOpType(ArithmeticLogicUnit::OT_SLTU);
                    ALU.setInput1(id_ex.A);
                    ALU.setInput2(id_ex.B);
                    uint32_t tmp = ALU.getOutput();
                    ALU.setInput1(id_ex.B);
                    ALU.setInput2(id_ex.A);
                    ALU.setInput1(ALU.getOutput());
                    ALU.setInput2(tmp);
                    ALU.setOpType(ArithmeticLogicUnit::OT_AND);
                    token = ALU.getOutput();
                    break; 
                case BLT: case BGE:
                    ALU.setInput1(id_ex.A);
                    ALU.setInput2(id_ex.B);
                    ALU.setOpType(ArithmeticLogicUnit::OT_SLT);
                    token = ALU.getOutput();
                    break;
                case BLTU: case BGEU:
                    ALU.setInput1(id_ex.A);
                    ALU.setInput2(id_ex.B);
                    ALU.setOpType(ArithmeticLogicUnit::OT_SLTU);
                    token = ALU.getOutput();
                    break;
            }
            switch (operation) {
                case BNE: case BGE: case BGEU: token = not token;
            }
            BP.set(token);
            if (token) {
                ALU.setInput1(id_ex.PC);
                ALU.setInput2(id_ex.B);
                pc = ALU.getOutput();
            }
            if (pc != id_ex.NPC) 
                this->throw_hazard();
            break;
        case OC_LOAD: case OC_STORE:
            ALU.setInput1(id_ex.A);
            ALU.setInput2(id_ex.B);
            ALU.setOpType(ArithmeticLogicUnit::OT_ADD);
            ex_mem.ALUOutput = ALU.getOutput();
            break;
        }
        if (IR.checkRd())
            FD.update_ex(IR.getRd(), ex_mem.ALUOutput);
        else 
            FD.update_ex();
        id_ex.clear();
    }
};

class MEMU : public StageUnit {
private:
    EX_MEM &ex_mem;
    MEM_WB &mem_wb;
    RandomAccessMemory &RAM;
    Forwarding &FD;
public:
    MEMU(
        EX_MEM &ex_mem, 
        MEM_WB &mem_wb, 
        RandomAccessMemory &RAM, 
        Forwarding &FD
    ) : ex_mem(ex_mem), mem_wb(mem_wb), RAM(RAM), FD(FD) {}
    void start() {
        StageUnit::start();
        if (ex_mem.IR == BUBBLE) return;
        mem_wb.IR = ex_mem.IR;
        InstructionDecoder IR(ex_mem.IR);
        switch (IR.getOpcode())
        {
        case OC_OP: case OC_OP_IMM: case OC_LUI: case OC_AUIPC: case OC_JAL: case OC_JALR:
            mem_wb.ALUOutput = ex_mem.ALUOutput;
            break;
        case OC_BRANCH:
            break;
        case OC_LOAD:
            set_clk_cyc(3); // LOAD 指令需要运行三个时钟周期
            switch (IR.getOperation()) {
                case LB: mem_wb.LMD = (int)(int8_t)RAM.read(ex_mem.ALUOutput);
                    break;
                case LH: mem_wb.LMD = (int)(int16_t)(RAM.read(ex_mem.ALUOutput) | RAM.read(ex_mem.ALUOutput + 1) << 8);
                    break;
                case LW: mem_wb.LMD = RAM.read(ex_mem.ALUOutput) | RAM.read(ex_mem.ALUOutput + 1) << 8 | RAM.read(ex_mem.ALUOutput + 2) << 16 | RAM.read(ex_mem.ALUOutput + 3) << 24;
                    break;
                case LBU: mem_wb.LMD = RAM.read(ex_mem.ALUOutput);
                    break;
                case LHU: mem_wb.LMD = RAM.read(ex_mem.ALUOutput) | RAM.read(ex_mem.ALUOutput + 1) << 8;
                    break;
            }
            break;
        case OC_STORE:
            set_clk_cyc(3); // STORE 指令需要运行三个时钟周期
            mem_wb.IR = ex_mem.IR;
            switch (IR.getOperation()) {
                case SW:
                    RAM.write(ex_mem.ALUOutput + 3, ex_mem.B >> 24);
                    RAM.write(ex_mem.ALUOutput + 2, ex_mem.B >> 16);
                case SH:
                    RAM.write(ex_mem.ALUOutput + 1, ex_mem.B >> 8);
                case SB:
                    RAM.write(ex_mem.ALUOutput, ex_mem.B);
                    break;
            }
            break;
        }
        ex_mem.clear();
    }
    void run() {
        StageUnit::run();
        if (IR.checkRd())
            FD.update_mem(IR.getRd(), mem_wb.ALUOutput);
        else 
            FD.update_mem();
    }
};

class WBU : public StageUnit {
private:
    MEM_WB &mem_wb;
    Registers &Regs;
public:
    WBU(MEM_WB &mem_wb, Registers &Regs) : mem_wb(mem_wb), Regs(Regs) {}
    void start() {
        StageUnit::start();
        if (mem_wb.IR == BUBBLE) return;
        InstructionDecoder IR(mem_wb.IR);
        switch (IR.getOpcode())
        {
        case OC_OP: case OC_OP_IMM: case OC_LUI: case OC_AUIPC: 
            Regs.write(IR.getRd(), mem_wb.ALUOutput);
            break;
        case OC_JAL: case OC_JALR: case OC_BRANCH:
            break;
        case OC_LOAD:
            Regs.write(IR.getRd(), mem_wb.LMD);
            break;
        case OC_STORE:
            break;
        }
        mem_wb.clear();
    }
};


class Instruction {

};

class PipeLine {
    
};



class CPU {
private:
    RandomAccessMemory &RAM;
    Registers RegisterFile;
    uint32_t pc;
    uint64_t clk;
    IF_ID if_id;
    ID_EX id_ex;
    EX_MEM ex_mem;
    MEM_WB mem_wb;
    BranchPredictor BP;
    Forwarding FD;
    int to_hex(char c) {
        if (c >= '0' && c <= '9')
            return c - '0';
        if (c >= 'a' && c <= 'f')
            return c - 'a' + 10;
        if (c >= 'A' && c <= 'F')
            return c - 'A' + 10;
        return EOF;
    }
public:
    CPU(RandomAccessMemory RAM) : RAM(RAM) {}
    void load_program() {
        char c = getchar();
        uint now_ptr = 0;
        while (true) {
            if (c == EOF) {
                break;
            }
            else if (c == '@') {
                now_ptr = 0;
                c = getchar();
                while (c == '0' || c == '1')
                    now_ptr = now_ptr << 1 | (c - '0'), c = getchar();
            } else {
                RAM.write(now_ptr, to_hex(c) << 4 | to_hex(getchar()));
                c = getchar();
            }
            c = getchar();
        }
    }/*
    void run() {
        pc = 0;
        clk = 0;
        IFU IF(pc, if_id, RAM, BP);
        IDU ID(pc, if_id, id_ex, RegisterFile, BP);
        EXU EX(id_ex, ex_mem, BP);
        MEMU MEM(ex_mem, mem_wb, RAM);
        WBU WB(mem_wb, RegisterFile);
        while (true) {
            IF.start();
            while (!IF.is_finished())
                IF.clk();

            ID.start();
            while (!ID.is_finished())
                ID.clk();
            
            EX.start();
            while (!EX.is_finished())
                EX.clk();

            MEM.start();
            while (!MEM.is_finished())
                MEM.clk();

            WB.start();
            while (!WB.is_finished())
                WB.clk();                
        }

    }*/
    void run_with_pipeline() {
        pc = 0;
        clk = 0;
        IFU IF(pc, if_id, RAM, BP);
        IDU ID(pc, if_id, id_ex, RegisterFile, BP, FD);
        EXU EX(pc, id_ex, ex_mem, BP, FD);
        MEMU MEM(ex_mem, mem_wb, RAM, FD);
        WBU WB(mem_wb, RegisterFile);
        while (true) {
            clk++;
            IF.run();
            ID.run();
            EX.run();
            MEM.run();
            WB.run();

            if (!WB.is_finished()) continue;
            else {
                WB.start();
                if (WB.catch_hazard()) {
                    WB.solve_hazard();
                    continue;
                }
            }
            
            if (!MEM.is_finished()) continue;
            else {
                MEM.start();
                if (MEM.catch_hazard()) {
                    MEM.solve_hazard();
                    continue;
                }
            }

            if (!EX.is_finished()) continue;
            else {
                EX.start();
                if (EX.catch_hazard()) {
                    EX.solve_hazard();
                    continue;
                }
            }

            if (!ID.is_finished()) continue;
            else {
                ID.start();
                if (ID.catch_hazard()) {
                    ID.solve_hazard();
                    continue;
                }
            }

            if (!IF.is_finished()) continue;
            else {
                IF.start();
                if (IF.catch_hazard()) {
                    IF.solve_hazard();
                    continue;
                }
            }
        }
    }
};

class Stall {

};

}

int main() {
    sjtu::RandomAccessMemory Mem;
1w  wwe1    Wu.load_program();
    cpu.run_with_pipeline();
}