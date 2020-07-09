#include <iostream>
using namespace std;

namespace sjtu {

class BranchPredictor {

};

class ALU {

}

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

class RISC_V {
private:
    uint8_t * Mem;
    uint32_t Regs[32], PC;

    struct IF_ID_t { uint32_t IR; uint32_t NPC; uint32_t PC; } IF_ID;
    struct ID_EX_t { uint32_t IR; uint32_t NPC; uint32_t PC; uint32_t A; uint32_t B; int32_t Imm; } ID_EX;
    struct EX_MEM_t { uint32_t IR; uint32_t ALUOutput; uint32_t B; bool cond; } EX_MEM;
    struct MEM_WB_t { uint32_t IR; uint32_t ALUOutput; uint32_t LMD; } MEM_WB;
    
    
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
    enum InstructionFormat {
        R_type, I_type, S_type, B_type, U_type, J_type
    };
    class InstructionDecoder {
    private:
        uint32_t IR;
        InstructionDecoder(uint32_t IR) : IR(IR) {}
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
    public:
        InstructionOpcode getOpcode()    { return InstructionOpcode(substr_of_inst(0, 7)); }
        uint32_t getRd()        { return substr_of_inst(7, 5); }
        uint32_t getFunct3()    { return substr_of_inst(12, 3); }
        uint32_t getRs1()       { return substr_of_inst(15, 5); }
        uint32_t getRs2()       { return substr_of_inst(20, 5); }
        uint32_t getShamt()     { return substr_of_inst(20, 5); }
        uint32_t getFunct7()    { return substr_of_inst(25, 7); }
        uint32_t getImm() {
            InstructionFormat fmt = getFormat();
            switch (fmt) {
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
    };
    
    void InstructionFetch(uint32_t &PC, IF_ID_t &IF_ID, EX_MEM_t EX_MEM) {
        InstructionDecoder IRdecoder(EX_MEM.IR);
        IF_ID.IR = (uint)Mem[PC] | (uint)Mem[PC + 1] << 8 | (uint)Mem[PC + 2] << 16 | (uint)Mem[PC + 3] << 24;
        IF_ID.PC = PC;
        IF_ID.NPC = PC = (IRdecoder.getOpcode == BRANCH && EX_MEM.cond ? EX_MEM.ALUOutput : PC + 4);
    }
    void InstructionDecode(IF_ID_t &IF_ID, ID_EX_t &ID_EX, uint32_t * Regs) {
        InstructionDecoder IRdecoder(IF_ID.IR);
        ID_EX.A = Regs[IRdecoder.getRs1()];
        ID_EX.B = Regs[IRdecoder.getRs2()];
        ID_EX.NPC = IF_ID.NPC;
        ID_EX.IR = IF_ID.IR;
        ID_EX.Imm = int32_t(IRdecoder.getImm());
    }
    void Execute(ID_EX_t &ID_EX, EX_MEM_t &EX_MEM) {
        InstructionDecoder IRdecoder(ID_EX.IR);
        switch (IRdecoder.getOpcode())
        {
        case OC_OP_IMM: 
            if (IRdecoder.getFunct3() == 0b001 or IRdecoder.getFunct3() == 0b101) 
                ID_EX.B = IRdecoder.getShamt();
            else 
                ID_EX.B = ID_EX.Imm;
        case OC_OP: 
            switch (IRdecoder.getFunct3())
            {
            case 0b000: // ADD SUB
                if (IRdecoder.getFunct7() == 0)
                    EX_MEM.ALUOutput = ID_EX.A + ID_EX.B;
                else 
                    EX_MEM.ALUOutput = ID_EX.A - ID_EX.B;
                break;
            case 0b001: // SLL
                EX_MEM.ALUOutput = ID_EX.A << ID_EX.B; break;
            case 0b010: // SLT
                EX_MEM.ALUOutput = (int32_t)ID_EX.A < (int32_t)ID_EX.B ? 1 : 0; break;
            case 0b011: // SLTU
                EX_MEM.ALUOutput = ID_EX.A < ID_EX.B ? 1 : 0; break;
            case 0b100: // XOR
                EX_MEM.ALUOutput = ID_EX.A ^ ID_EX.B; break;
            case 0b101: // SRA SRL
                if (IRdecoder.getFunct7() == 0)
                    EX_MEM.ALUOutput = ID_EX.A >> ID_EX.B;               
                else
                    EX_MEM.ALUOutput = (uint32_t)ID_EX.A >> ID_EX.B;
                break;
            case 0b110: // OR
                EX_MEM.ALUOutput = ID_EX.A | ID_EX.B; break;
            case 0b111: // AND
                EX_MEM.ALUOutput = ID_EX.A & ID_EX.B; break;
            default:
                break;
            }
            break;
        case OC_LUI: 
            EX_MEM.ALUOutput = (uint32_t)ID_EX.Imm >> 12; break;
        case OC_AUIPC: 
            EX_MEM.ALUOutput = ((uint32_t)ID_EX.Imm >> 12) + ID_EX.PC; break;
            break;
        case OC_JAL: 
            EX_MEM.ALUOutput = ID_EX.PC + 4; break;
            EX_MEM.NPC = 
        case OC_JALR: 
        case OC_BRANCH:
            break;
        case OC_LOAD: case OC_STORE:
            break;
        }
    }

    void MemoryAccess(EX_MEM_t &EX_MEM, MEM_WB_t &MEM_WB, uint_8 * Mem) {
        InstructionDecoder IRdecoder(EX_MEM.IR);
        switch (IRdecoder.getOpcode())
        {
        case OC_OP: case OC_OP_IMM: case OC_LUI: case OC_AUIPC: case OC_JAL: case OC_JALR:
            MEM_WB.IR = EX_MEM.IR;
            MEM_WB.ALUOutput = EX_MEM.ALUOutput;
            break;
        case OC_BRANCH:
            break;
        case OC_LOAD:
            MEM_WB.IR = EX_MEM.IR; 
            switch (IRdecoder.getOperation()) {
                case LB: MEM_WB.LMD = (int)(int8_t)Mem[EX_MEM.ALUOutput];
                    break;
                case LH: MEM_WB.LMD = (int)(int16_t)(Mem[EX_MEM.ALUOutput] | Mem[EX_MEM.ALUOutput + 1] << 8);
                    break;
                case LW: MEM_WB.LMD = Mem[EX_MEM.ALUOutput] | Mem[EX_MEM.ALUOutput + 1] << 8 |  Mem[EX_MEM.ALUOutput + 2] << 16 |  Mem[EX_MEM.ALUOutput + 3] << 24;
                    break;
                case LBU: MEM_WB.LMD = Mem[EX_MEM.ALUOutput];
                    break;
                case LHU: MEM_WB.LMD = Mem[EX_MEM.ALUOutput] | Mem[EX_MEM.ALUOutput + 1] << 8;
                    break;
            }
            break;
        case OC_STORE:
            MEM_WB.IR = EX_MEM.IR;
            switch (IRdecoder.getOperation()) {
                case SW:
                    Mem[EX_MEM.ALUOutput + 3] = EX_MEM.B >> 24;
                    Mem[EX_MEM.ALUOutput + 2] = EX_MEM.B >> 16;
                case SH:
                    Mem[EX_MEM.ALUOutput + 1] = EX_MEM.B >> 8;
                case SB:
                    Mem[EX_MEM.ALUOutput] = EX_MEM.B;
                    break;
            }
            break;
        }
    }

    void WriteBack(MEM_WB_t &MEM_WB, uint32_t * Regs) {
        InstructionDecoder IRdecoder(MEM_WB.IR);
        switch (IRdecoder.getOpcode())
        {
        case OC_OP: case OC_OP_IMM: case OC_LUI: case OC_AUIPC: 
            Regs[IRdecoder.getRd()] = MEM_WB.ALUOutput;
            break;
        case OC_JAL: case OC_JALR: case OC_BRANCH:
            break;
        case OC_LOAD:
            Regs[IRdecoder.getRd()] = MEM_WB.LMD;
            break;
        case OC_STORE:
            break;
        }
    }

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
    RISC_V() {
        Mem = new uint8_t[1 << 20];
    }
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
                Mem[now_ptr] = to_hex(c) << 4 | to_hex(getchar());
                c = getchar();
            }
            c = getchar();
        }
    }
    void run() {
        PC = 0;
        while (true) {
            
        }
    }
    ~RISC_V() {
        delete [] Mem;
    }
};

}

int main() {
    sjtu::RISC_V cpu;
    cpu.load_program();
    cpu.run();
}
