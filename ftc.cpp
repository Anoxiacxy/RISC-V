#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdio>
#include <cstring>
struct move{};
struct end{
    int num;
    end(int n=0):num(n){}
};
inline bool is_num(char c){
    if(c>='0'&&c<='9') return true;
    if(c>='A'&&c<='F') return true;
    if(c=='@') throw move();
    if(c==EOF) throw end();
    return false;
}
inline void read(unsigned int& num){
    char ch=getchar();unsigned int x=0;
    while(!is_num(ch)){ch=getchar();}
    while(is_num(ch)){
        ch = (ch<'A')?(ch-'0'):(ch-'A'+10);
        x=(x<<4)+ch;ch=getchar();
    }
    num = x;
}
inline unsigned int read(){//把32位码转换成int
    char ch[8];
    ch[1] = getchar();
    while(!is_num(ch[1])){ch[1]=getchar();}
                      ch[0] = getchar();getchar();
    ch[3] = getchar();ch[2] = getchar();getchar();
    ch[5] = getchar();ch[4] = getchar();getchar();
    ch[7] = getchar();ch[6] = getchar();

    // printf("read ->");for(int i=7;i>=0;--i) putchar(ch[i]);puts("");

    unsigned int ans=0;
    for(int i=0;i<8;i++){
        if(ch[i]<'A') ch[i] = ch[i]-'0';
        else ch[i] = ch[i]-'A'+10;
        ans |= ((unsigned int)(ch[i]) << (i<<2) );
        // printf("ans |= %d << %d\n",ch[i],4*i);
    }
    // printf("return %d\n",ans);
    return ans;
}
struct error{
    unsigned int line,num;
    error(unsigned int l,unsigned int n):line(l),num(n){}
    void printerror(){
        std::cerr<<"ERROR in line "<<line<<"!\tnum = "<<num<<std::endl;
    }
};
const char* name[] = {"LUI","AUIPC","JAL","JALR","BEQ","BNE","BLT","BGE","BLTU","BGEU","LB","LH","LW","LBU","LHU","SB","SH","SW","ADDI","SLTI","SLTIU","XORI","ORI","ANDI","SLLI","SRLI","SRAI","ADD","SUB","SLL","SLT","SLTU","XOR","SRL","SRA","OR","AND"};
const char* REGI_name[] = {"zero","ra","sp","qp","tp","t0","t1","t2","s0","s1","a0","a1","a2","a3","a4","a5","a6","a7","s2","s3","s4","s5","s6","s7","s8","s9","s10","s11","t3","t4","t5","t6"};
enum INSTRUCTION{
    LUI,      //U
    AUIPC,    //U  
    JAL,      //J
    JALR,     //I
    BEQ,      //B
    BNE,      //B
    BLT,      //B
    BGE,      //B
    BLTU,     //B
    BGEU,     //B
    LB,       //I
    LH,       //I
    LW,       //I
    LBU,      //I
    LHU,      //I
    SB,       //S
    SH,       //S
    SW,       //S
    ADDI,     //I
    SLTI,
    SLTIU,
    XORI,
    ORI,
    ANDI,
    SLLI,
    SRLI,
    SRAI,
    ADD,      //R
    SUB,
    SLL,
    SLT,
    SLTU,
    XOR,
    SRL,
    SRA,
    OR,
    AND
};
enum INSTRUCTION_TYPE{_U,_J,_B,_I,_S,_R};
unsigned int REGI[32];//模拟寄存器
unsigned int PC;//内存指针
const int MAXm=1<<16;
int MEM[MAXm];//模拟计算机内存
struct instructions{
    bool initialized;//是否初始化
    unsigned int n;
    INSTRUCTION instr;
    instructions():n(0),instr(LUI),initialized(false){}
    instructions(const instructions&other):
    n(other.n),instr(other.instr),initialized(other.initialized){}

    void set(unsigned int num){
        this->initialized=true;
        this->n = num;
        if(num%4 != 0x3) throw error(__LINE__,num);
        unsigned int sub_num1 = (num>>12)%8;
        if( (num>>2)%2 == 0x1) {//                      111
            if((num>>3)%2 == 0x1){//                   1111
                this->instr = JAL;
            } else {//                                 0111
                if((num>>4)%2== 0x1){//               10111
                    if((num>>5)%2 == 0x1){//         010111
                        this->instr = LUI;
                    }else {//                        110111
                        this->instr = AUIPC;
                    }
                } else{//                             00111
                    this->instr = JALR;
                }
            }
        }else{//                                        011
            if((num>>3)%2 == 0x1) throw error(__LINE__,num);//     1011
            if((num>>4)%2 == 0x1){//                  10011
                if((num>>6)%2 == 0x1) throw error(__LINE__,num);
                unsigned int sub_num2 = (num>>30);
                if((num>>5)%2 == 0x1){//            0110011
                    switch (sub_num1){
                        case 0x0:
                            if(sub_num2==0x1) this->instr = SUB;
                            else this->instr = ADD;
                        break;
                        case 0x1: this->instr = SLL;  break;
                        case 0x2: this->instr = SLT;  break;
                        case 0x3: this->instr = SLTU; break;
                        case 0x4: this->instr = XOR;  break;
                        case 0x5:
                            if(sub_num2==0x1) this->instr = SRA;
                            else this->instr = SRL;
                        break;
                        case 0x6: this->instr = OR;   break;
                        case 0x7: this->instr = AND;  break;
                        default: throw error(__LINE__,num);       break;
                    }
                }else{//                            0010011
                    switch (sub_num1){
                        case 0x0: this->instr = ADDI; break;
                        case 0x1: this->instr = SLLI; break;
                        case 0x2: this->instr = SLTI; break;
                        case 0x3: this->instr = SLTIU;break;
                        case 0x4: this->instr = XORI; break;
                        case 0x5:
                            if(sub_num2==0x1) this->instr = SRAI;
                            else this->instr = SRLI;
                        break;
                        case 0x6: this->instr = ORI;  break;
                        case 0x7: this->instr = ANDI; break;
                        default: throw error(__LINE__,num);       break;
                    }
                }
            }else{//                                  00011
                if((num>>5)%2 == 0x1){//             100011
                    if((num>>6)%2 == 0x1){//            1100011
                        switch (sub_num1){//             1100011
                            case 0x0: this->instr = BEQ;  break;
                            case 0x1: this->instr = BNE;  break;
                            case 0x4: this->instr = BLT;  break;
                            case 0x5: this->instr = BGE;  break;
                            case 0x6: this->instr = BLTU; break;
                            case 0x7: this->instr = BGEU; break;
                            default:  throw error(__LINE__,num);      break;
                        }
                    }else{//                            0100011
                        switch (sub_num1){
                            case 0x0: this->instr = SB;   break;
                            case 0x1: this->instr = SH;   break;
                            case 0x2: this->instr = SW;   break;
                            default:  throw error(__LINE__,num);      break;
                        }
                    }
                }else{//                             000011
                    if((num>>6)%2 == 0x1) throw error(__LINE__,num);
                    switch (sub_num1){//             0000011
                        case 0x0: this->instr = LB;   break;
                        case 0x1: this->instr = LH;   break;
                        case 0x2: this->instr = LW;   break;
                        case 0x4: this->instr = LBU;  break;
                        case 0x5: this->instr = LHU;  break;
                        default:  throw error(__LINE__,num);      break;
                    }
                }//end 000011
            }//end 00011
        }//end 011
    }//end function

    void print_name(){
        std::cout<<name[this->instr]<<std::endl;
    }

    INSTRUCTION_TYPE type(){
        if(!initialized) throw error(__LINE__,0);
        if(this->instr>=ADD) return _R;
        if(this->instr<=AUIPC) return _U;
        if(this->instr==JAL) return _J;
        if(this->instr>=BEQ&&this->instr<=BGEU) return _B;
        if(this->instr>=SB&&this->instr<=SW) return _S;
        return _I;
    }
    virtual void execute(){throw error(__LINE__,0);}
    virtual void show(){throw error(__LINE__,0);}
};//end class

struct U:public instructions{
    unsigned int imm;//立即数
    unsigned short rd;//寄存器位置
    U(const instructions& in):instructions(in),imm(0),rd(0){
        this->rd = (this->n>>7)%32;
        this->imm = (this->n & 0xFFFFF000UL);//不用移位
    }
    void lui(){
        REGI[this->rd] = this->imm;
    }//imm貌似已经移过了
    void auipc(){
        REGI[this->rd] += this->imm;
    }//imm貌似已经移过了
    void show()override{
        this->print_name();
        printf("PC = 0x%x\n",PC);
        printf("%s,imm = 0x%x or %d\n",REGI_name[rd],imm,imm);
    }
    void execute()override{
        this->show();
        switch(this->instr){
            case LUI:  this->lui();  break;
            case AUIPC:this->auipc();break;
            default: throw error(__LINE__,0); break;
        }
    }
};

struct J:public instructions{
    unsigned int imm;//立即数
    unsigned short rd;//寄存器位置
    J(const instructions& in):instructions(in),imm(0){
        this->rd = (this->n>>7)%32;

        if ((this->n >> 31u) == 1u)
            imm |= 0xfff00000UL;
        imm |= this->n & 0x000ff000UL;
        imm |= ((this->n >> 20u) & 1u) << 11u;
        imm |= ((this->n >> 21u) & 1023u) << 1u;
    }
    void jal(){
        REGI[this->rd]=PC+4UL;
        PC+=this->imm;
        PC-=4;//补偿
    }
    void show()override{
        this->print_name();
        printf("%s imm = 0x%x or %d\n",REGI_name[rd],imm,imm);
    }
    void execute()override{
        this->show();
        this->jal();
    }
};

struct B:public instructions{
    unsigned short rs1;//寄存器
    unsigned short rs2;//寄存器
    unsigned int imm;//立即数
    B(const instructions& in):instructions(in),rs1(0),rs2(0),imm(0){
        this->rs1 = (this->n>>15)%32;
        this->rs2 = (this->n>>20)%32;
        if ((this->n >> 31u) == 1u)
            imm |= 0xfffff000;
        imm |= ((this->n >> 7u) & 1u) << 11u;
        imm |= ((this->n >> 25u) & 63u) << 5u;
        imm |= ((this->n >> 8u) & 15u) << 1u;
    }
    void show()override{
        this->print_name();
        printf("PC = 0x%x\n",PC);
        printf("%s  %s,imm = 0x%x or %d\n",REGI_name[rs1],REGI_name[rs2],imm,imm);
    }
    void beq(){if(REGI[this->rs1]==REGI[this->rs2]) {PC += this->imm;if(this->imm%4!=0) throw error(__LINE__,0);} }
    void bne(){if(REGI[this->rs1]!=REGI[this->rs2]) {PC += this->imm;if(this->imm%4!=0) throw error(__LINE__,0);} }
    void blt(){if((int)REGI[this->rs1]< (int)REGI[this->rs2]) {PC += this->imm;if(this->imm%4!=0) throw error(__LINE__,0);} }
    void bge(){if((int)REGI[this->rs1]>=(int)REGI[this->rs2]) {PC += this->imm;if(this->imm%4!=0) throw error(__LINE__,0);} }
    void bltu(){if(REGI[this->rs1]< REGI[this->rs2]) {PC += this->imm;if(this->imm%4!=0) throw error(__LINE__,0);} }
    void bgeu(){if(REGI[this->rs1]>=REGI[this->rs2]) {PC += this->imm;if(this->imm%4!=0) throw error(__LINE__,0);} }
    void execute()override{
        this->show();
        switch(this->instr){
            case BEQ:  this->beq();  break;
            case BNE:  this->bne();  break;
            case BLT:  this->blt();  break;
            case BGE:  this->bge();  break;
            case BLTU: this->bltu(); break;
            case BGEU: this->bgeu(); break;
            default: throw error(__LINE__,0); break;
        }
    }
};

struct I:public instructions{
    unsigned short rs1;//寄存器
    unsigned short rd;//寄存器
    unsigned int imm;//立即数
    // unsigned short shamt=0;
    I(const instructions& in):instructions(in),rs1(0),rd(0),imm(0){
        this->rs1 = (this->n>>15)%32;
        this->rd = (this->n>>7)%32;
        // this->shamt = (this->n>>20)%32;
        if ((this->n >> 31u) == 1u)
            imm |= 0xfffff800;
        imm |= (this->n >> 20u) & 2047u;
    }
    void show()override{
        this->print_name();
        printf("PC = 0x%x\n",PC);
        printf("%s  %s,imm = 0x%x or %d\n",REGI_name[rd],REGI_name[rs1],imm,imm);
    }
    void jalr(){
        REGI[this->rd] = PC+4;
        PC = (this->imm+REGI[this->rs1])&(-2u);
        PC -= 4;
    }
    void lb(){
        // REGI[this->rd]=reinterpret_cast<char*>(MEM)[REGI[this->rs1] +(int)this->imm];
        // if((REGI[this->rd]>>7)%2==0x0) return;
        // REGI[this->rd] |= 0xFFFFFF00UL;
        char ch;
        memcpy(&ch,(char*)MEM+(REGI[rs1]+imm),sizeof(char));
        REGI[rd]=(unsigned int)ch;
    }
    void lh(){
        // REGI[this->rd]=reinterpret_cast<short*>(MEM)[(REGI[this->rs1] +(int)this->imm)/2];
        // if((REGI[this->rd]>>15)%2==0x0) return;
        // REGI[this->rd] |= 0xFFFF0000UL;
        short ch;
        memcpy(&ch,(char*)MEM+(REGI[rs1]+imm),sizeof(short));
        REGI[rd]=(unsigned int)ch;
    }
    void lw(){
        // REGI[this->rd]=MEM[(REGI[this->rs1] +(int)this->imm)/4];
        memcpy(&REGI[rd],(char*)MEM+(REGI[rs1]+imm),sizeof(unsigned int));
    }
    void lbu(){
        // REGI[this->rd]=reinterpret_cast<char*>(MEM)[REGI[this->rs1] +(int)this->imm];
        unsigned char ch;
        memcpy(&ch,(char*)MEM+(REGI[rs1]+imm),sizeof(unsigned char));
        REGI[rd]=(unsigned int)ch;
    }
    void lhu(){
        // REGI[this->rd]=reinterpret_cast<short*>(MEM)[(REGI[this->rs1] +(int)this->imm)/2];
        unsigned short ch;
        memcpy(&ch,(char*)MEM+(REGI[rs1]+imm),sizeof(unsigned short));
        REGI[rd]=(unsigned int)ch;
    }
    void addi(){
        if(this->rd==10&&this->imm==0xFF)
            throw end(REGI[rd]&0xFFUL);
        REGI[this->rd]=REGI[this->rs1]+this->imm;
    }
    void slti(){REGI[this->rd]=((int)REGI[this->rs1]<(int)this->imm);}
    void sltiu(){REGI[this->rd]=(REGI[this->rs1]<this->imm);}
    void xori(){REGI[this->rd]=REGI[this->rs1]^this->imm;}
    void ori(){REGI[this->rd]=REGI[this->rs1]|this->imm;}
    void andi(){REGI[this->rd]=REGI[this->rs1]&this->imm;}

    void slli(){REGI[this->rd]=REGI[this->rs1]<<(imm&31UL);}
    void srli(){REGI[this->rd]=REGI[this->rs1]>>(imm&31UL);}
    void srai(){
        REGI[rd]=(int) REGI[rs1]>>(imm&31UL);
    }
    void execute()override{
        this->show();
        switch(this->instr){
            case JALR: this->jalr(); break;
            case LB:   this->lb();   break;
            case LH:   this->lh();   break;
            case LW:   this->lw();   break;
            case LBU:  this->lbu();  break;
            case LHU:  this->lhu();  break;
            case ADDI: this->addi(); break;
            case SLTI: this->slti(); break;
            case SLTIU:this->sltiu();break;
            case XORI: this->xori(); break;
            case ORI:  this->ori();  break;
            case ANDI: this->andi(); break;
            case SLLI: this->slli(); break;
            case SRLI: this->srli(); break;
            case SRAI: this->srai(); break;
            default: throw error(__LINE__,0); break;
        }
    }
};

struct S:public instructions{
    unsigned short rs1;//寄存器
    unsigned short rs2;//寄存器
    unsigned int imm;//立即数
    S(const instructions& in):instructions(in),rs1(0),rs2(0),imm(0){
        this->rs1 = (this->n>>15)%32;
        this->rs2 = (this->n>>20)%32;
        if ((this->n >> 31u) == 0x1u)
            imm |= 0xfffff800;
        imm |= ((this->n >> 25u) & 63u) << 5u;
        imm |= ((this->n >> 8u) & 15u) << 1u;
        imm |= (this->n >> 7u) & 1u;
    }
    void show()override{
        this->print_name();
        printf("PC = 0x%x\n",PC);
        printf("%s  %s,imm = 0x%x or %d\n",REGI_name[rs1],REGI_name[rs2],imm,imm);
    }
    void sb(){
        char ch=(char) REGI[rs2];
        memcpy((char*)MEM+(REGI[rs1]+(int)imm),&ch,sizeof(char));
    }
    void sh(){
        short ch=(unsigned short) REGI[rs2];
        memcpy((char*)MEM+(REGI[rs1]+(int)imm),&ch,sizeof(unsigned short));
    }
    void sw(){
        memcpy((char*)MEM+(REGI[rs1]+imm),&REGI[rs2],sizeof(unsigned int));
    }
    void execute()override{
        this->show();
        switch(this->instr){
            case SB:   this->sb();   break;
            case SH:   this->sh();   break;
            case SW:   this->sw();   break;
            default: throw error(__LINE__,0); break;
        }
    }
};

struct R:public instructions{
    unsigned short rs1;//寄存器
    unsigned short rs2;//寄存器
    unsigned short rd;//寄存器
    R(const instructions& in):instructions(in),rs1(0),rs2(0),rd(0){
        this->rs1 = (this->n>>15)%32;
        this->rs2 = (this->n>>20)%32;
        this->rd  = (this->n>>7)%32;
    }
    void show()override{
        this->print_name();
        printf("PC = 0x%x\n",PC);
        printf("%s  %s  %s\n",REGI_name[rd],REGI_name[rs1],REGI_name[rs2]);
    }
    void add(){REGI[this->rd]=REGI[this->rs1]+REGI[this->rs2];}
    void sub(){REGI[this->rd]=REGI[this->rs1]-REGI[this->rs2];}

    void sll(){REGI[this->rd]=REGI[this->rs1]<<(REGI[this->rs2]&31UL);}//逻辑左移

    void slt(){REGI[this->rd]=((int)REGI[this->rs1]<(int)REGI[this->rs2]);}
    void sltu(){REGI[this->rd]=(REGI[this->rs1]<REGI[this->rs2]);}

    void _xor(){REGI[this->rd]=REGI[this->rs1]^REGI[this->rs2];}
    void srl(){//逻辑右移(空位补0)
        REGI[this->rd]=REGI[this->rs1]>>(REGI[this->rs2]&31UL);
    }
    void sra(){//算数右移(空位用最高位填充)
        REGI[rd]=(int)(REGI[rs1])>>(REGI[rs2]&31UL);
    }
    void _or(){REGI[this->rd]=REGI[this->rs1]|REGI[this->rs2];}
    void _and(){REGI[this->rd]=REGI[this->rs1]&REGI[this->rs2];}
    void execute()override{
        this->show();
        switch (this->instr){
        case ADD:  this->add();  break;
        case SUB:  this->sub();  break;
        case SLL:  this->sll();  break;
        case SLT:  this->slt();  break;
        case SLTU: this->sltu(); break;
        case XOR:  this->_xor();  break;
        case SRL:  this->srl();  break;
        case SRA:  this->sra();  break;
        case OR:   this->_or();   break;
        case AND:  this->_and();  break;
        default: throw error(__LINE__,0); break;
        }
    }
};

int main(){
    //freopen("./testcases/array_test1.data","r",stdin);
    // freopen(".\\testcases\\array_test1.data","r",stdin);
    flag1:
    try{
        while (true){
            MEM[PC/4]=read();
            // instructions tmp,*ptr;
            // // tmp.set(MEM[PC/4]);
            // tmp.set(read());
            // // tmp.print_name();
            // // PC+=4;

            // switch(tmp.type()){
            //     case _U: ptr = new U(tmp);break;
            //     case _J: ptr = new J(tmp);break;
            //     case _B: ptr = new B(tmp);break;
            //     case _I: ptr = new I(tmp);break;
            //     case _S: ptr = new S(tmp);break;
            //     case _R: ptr = new R(tmp);break;
            //     default: throw error(__LINE__,0);break;
            // }
            // ptr->show();
            // puts("=====================================================");
            // delete ptr;
            PC+=4;
        }
    }
    catch(move e){
        read(PC);
        printf("move to 0x%x\n",PC);
        goto flag1;
    }
    catch(end e){
        puts("successfully read!");
        PC=0;
    }
    catch(error e){
        e.printerror();
        goto flag1;
    }

    instructions tmp;
    flag2:
    try{
        while(1){
            // printf("PC = %x\n",PC);
            tmp.set(MEM[PC/4]);
            instructions* ptr=NULL;
            switch(tmp.type()){
                case _U: ptr = new U(tmp);break;
                case _J: ptr = new J(tmp);break;
                case _B: ptr = new B(tmp);break;
                case _I: ptr = new I(tmp);break;
                case _S: ptr = new S(tmp);break;
                case _R: ptr = new R(tmp);break;
                default: throw error(__LINE__,0);break;
            }
            REGI[0]=0;
            ptr->execute();
            delete ptr;
            PC += 4;
            // REGI[0]=0UL;
            puts("====================================================");
        }
    }
    catch(error e){
        e.printerror();
        goto flag2;
    }
    catch(end e){
        printf("%d\n",e.num);
    }

    
    return 0;
}