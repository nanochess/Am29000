/*
 ** Am29000 processor emulator
 **
 ** by Oscar Toledo G.
 ** https://nanochess.org/
 **
 ** Creation date: Jul/30/2026.
 ** Revision date: Aug/03/2026. Added endianness change support.
 ** Revision date: Aug/13/2026. Solved bug in LOADM/STOREM when going from global
 **                             register to local register.
 ** Revision date: Sep/02/2026. Added support for Am29050 multiplication and
 **                             floating point instructions.
 */

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifdef __APPLE__
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#endif
#ifdef _WIN32
#include <winsock2.h>
#include <WS2tcpip.h>
#endif

#include "am29000.h"
#include "clgd5429.h"
#include "clgd5440.h"

#if AM29050
#include <math.h>
#endif

/*
 ** Am29000 manual from https://archive.org/details/bitsavers_amdAm29000ual_16568459
 ** Am29050 manual from https://archive.org/details/bitsavers_amdAm29000nual_30055159
 **   (for some reason not found if you search for am29050)
 **
 ** The original Am29000 manual has several deviations over its successor Am29050,
 ** in particular the floating-point instructions opcodes change, and the added
 ** multiplication instructions.
 **
 ** Section numbers cited are from the Am29000 manual.
 */

/*
 ** Not implemented instructions for a complete Am29000 processor:
 ** o LOADL 0x06 0x07
 ** o LOADSET 0x26 0x27
 ** o MFTLB 0xb6
 ** o MTTLB 0xbe
 ** o STOREL 0x0e 0x0f
 ** o Arithmetic traps (ADDS, ADDU, SUBS, SUBU)
 **
 ** Not implemented for a complete Am29000 processor:
 ** o Vector Base Address: Access per address (VF bit 4 of CFG)
 ** o Interruption pins (required if the 16550 serial chip is emulated), and the
 **   corresponding IP bit 14 and bits 3-1 in CPS register.
 ** o Single step trace (bits 13 and 12 of CPS register)
 ** o Unaligned trap handling (Trap-Unaligned bit set in bit 11 of CPS register)
 ** o WAIT mode (WM bit set in bit 7 of CPS)
 ** o Supervisor mode (bit 4 of CPS)
 ** o MMU handling (TLB registers, MMU and LRU registers, and bit 6 and 5 of CPS)
 ** o Channel registers.
 ** o Interruptable LOADM/STOREM (and registers CHA, CHD, and CHC)
 ** o Register protection (RBP register)
 **
 ** Not implemented for a complete Am29050 processor:
 ** o CLASS instruction (still processed through emulation)
 ** o Rounding modes for CONVERT instruction.
 ** o FMAC, DMAC, FMSM, DMSM, MTACC, and MFACC instructions (plus four accumulator registers)
 ** o FDMUL.
 ** o Registers gr2 and gr3 (Condition Code accumulators)
 ** o Special registers rsn, rma0, rmc0, rma1, rmc1, spc0, spc1, and spc2.
 ** o Breakpoint registers iba0, ibc0, iba1, ibc1.
 ** o Environment and status registers fpe, inte, fps.
 ** o Exception opcode exop.
 */

/*
 ** I should have defined macros for accesing these registers,
 ** here is a handy reference for access to special[] array
 ** (Am29000 manual)
 **
 ** Page 62:
 ** Vector Area Base Address (register 0)
 ** Old Processor Status (register 1)
 ** Current Processor Status (register 2)
 ** Configuration (register 3)
 ** Channel Address (register 4)
 ** Channel Data (register 5)
 ** Channel Control (register 6)
 ** Register Bank Protect (register 7)
 ** Timer Counter (register 8)
 ** Timer Reload (register 9)
 ** Program Counter 0 (register 10 decode) (next instruction to be executed)
 ** Program Counter 1 (register 11 execute) (current instruction)
 ** Program Counter 2 (register 12 write-back) (previous instruction)
 ** MMU Configuration (register 13)
 ** LRU Recommendation (register 14)
 ** Indirect Pointer C (register 128)
 ** Indirect Pointer A (register 129)
 ** Indirect Pointer B (register 130)
 ** Q (register 131)
 ** ALU Status (register 132)
 ** Byte Pointer (register 133)
 ** Funnel Shift Count (register 134)
 ** Load/Store Count Remaining (register 135)
 */

uint32_t rom[ROM_SIZE / 4];
uint32_t memory[RAM_SIZE / 4];
uint32_t regs[256];
uint32_t special[256];

uint32_t pc0;
uint32_t pc1;
uint32_t pc2;

uint32_t count;
uint32_t endianness;

/*
 ** Read a byte
 */
int read_byte(uint32_t addr)
{
    int c;

    c = (8 * ((addr ^ endianness) & 3));
    return (read_word(addr) >> c) & 0xff;
}

/*
 ** Write a byte
 */
void write_byte(uint32_t addr, uint32_t byte)
{
    uint32_t word;
    int c;
    
    c = (8 * ((addr ^ endianness) & 3));
    word = read_word(addr);
    word &= ~(0x000000ffu << c);
    word |= (byte & 0xff) << c;
    write_word(addr, word);
}

/*
 ** Count leading zeroes
 */
int clz(uint32_t value)
{
    unsigned char lz[256] = {
        8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    
    int c;
    
    c = 0;
    if (((value >> 24) & 0xff) == 0) {
        c += 8;
        value <<= 8;
        if (((value >> 24) & 0xff) == 0) {
            c += 8;
            value <<= 8;
            if (((value >> 24) & 0xff) == 0) {
                c += 8;
                value <<= 8;
            }
        }
    }
    return c + lz[value >> 24];
}

/*
 ** Handle trap
 */
void trap(int number)
{
    special[1] = special[2];    /* Page 3-55 s*/
    special[2] = (special[2] & 0xc10c) | 0x0473;    /* Figure 3-34 */
    special[10] = pc0;
    special[11] = pc1;
    special[12] = pc2;
    if ((special[3] & 0x10) == 0) {
        pc1 = (special[0] & 0xffff0000) | (number << 8);
        pc0 = pc1 + 4;
    } else {
        fprintf(stderr, "Vector Fetch table not implemented\n");
        exit(1);
    }
}

/*
 ** Special registers of the AMD Am29k processors.
 **
 ** The original Am29000 manual doesn't have official abbreviated names
 ** for the special registers. These names come from the Am29050
 ** manual.
 */
char *special_regs[] = {
    "vab", "ops", "cps", "cfg", "cha", "chd", "chc", "rbp",
    "tmc", "tmr", "pc0", "pc1", "pc2", "mmu", "lru", "rsn",
    "rma0", "rmc0", "rma1", "rmc1", "spc0", "spc1", "spc2", "iba0",
    "ibc0", "iba1", "ibc1", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "ipc", "ipa", "ipb", "q", "alu", "bp", "fc", "cr",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "fpe", "inte", "fps", "?", "exop", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
};

/*
** Disassemble an instruction
*/
void disassemble(uint32_t pc, uint32_t instruction, char *p)
{
    char *mnemonic[] = {
        "???", "CONSTN %a,%m", "CONSTH %a,%m", "CONST %a,%m", "MTSRIM %s,%m", "CONSTHZ %a,%m", "LOADL %l,%a,%b", "LOADL %l,%a,%i",
        "CLZ %c,%b", "CLZ %c,%i", "EXBYTE %c,%a,%b", "EXBYTE %c,%a,%i", "INBYTE %c,%a,%b", "INBYTE %c,%a,%i", "STOREL %l,%a,%b", "STOREL %l,%a,%i",
        "ADDS %c,%a,%b", "ADDS %c,%a,%i", "ADDU %c,%a,%b", "ADDU %c,%a,%i", "ADD %c,%a,%b", "ADD %c,%a,%i", "LOAD %l,%a,%b", "LOAD %l,%a,%i",
        "ADDCS %c,%a,%b", "ADDCS %c,%a,%i", "ADDCU %c,%a,%b", "ADDCU %c,%a,%i", "ADDC %c,%a,%b", "ADDC %c,%a,%i", "STORE %l,%a,%b", "STORE %l,%a,%i",
        "SUBS %c,%a,%b", "SUBS %c,%a,%i", "SUBU %c,%a,%b", "SUBU %c,%a,%i", "SUB %c,%a,%b", "SUB %c,%a,%i", "LOADSET %l,%a,%b", "LOADSET %l,%a,%i",
        "SUBCS %c,%a,%b", "SUBCS %c,%a,%i", "SUBCU %c,%a,%b", "SUBCU %c,%a,%i", "SUBC %c,%a,%b", "SUBC %c,%a,%i", "CPBYTE %c,%a,%b", "CPBYTE %c,%a,%i",
        "SUBRS %c,%a,%b", "SUBRS %c,%a,%i", "SUBRU %c,%a,%b", "SUBRU %c,%a,%i", "SUBR %c,%a,%b", "SUBR %c,%a,%i", "LOADM %l,%a,%b", "LOADM %l,%a,%i",
        "SUBRCS %c,%a,%b", "SUBRCS %c,%a,%i", "SUBRCU %c,%a,%b", "SUBRCU %c,%a,%i", "SUBRC %c,%a,%b", "SUBRC %c,%a,%i", "STOREM %l,%a,%b", "STOREM %l,%a,%i",
        "CPLT %c,%a,%b", "CPLT %c,%a,%i", "CPLTU %c,%a,%b", "CPLTU %c,%a,%i", "CPLE %c,%a,%b", "CPLE %c,%a,%i", "CPLEU %c,%a,%b", "CPLEU %c,%a,%i",
        "CPGT %c,%a,%b", "CPGT %c,%a,%i", "CPGTU %c,%a,%b", "CPGTU %c,%a,%i", "CPGE %c,%a,%b", "CPGE %c,%a,%i", "CPGEU %c,%a,%b", "CPGEU %c,%a,%i",
        "ASLT %t,%a,%b", "ASLT %t,%a,%i", "ASLTU %t,%a,%b", "ASLTU %t,%a,%i", "ASLE %t,%a,%b", "ASLE %t,%a,%i", "ASLEU %t,%a,%b", "ASLEU %t,%a,%i",
        "ASGT %t,%a,%b", "ASGT %t,%a,%i", "ASGTU %t,%a,%b", "ASGTU %t,%a,%i", "ASGE %t,%a,%b", "ASGE %t,%a,%i", "ASGEU %t,%a,%b", "ASGEU %t,%a,%i",
        "CPEQ %c,%a,%b", "CPEQ %c,%a,%i", "CPNEQ %c,%a,%b", "CPNEQ %c,%a,%i", "MUL %c,%a,%b", "MUL %c,%a,%i", "MULL %c,%a,%b", "MULL %c,%a,%i",
        "DIV0 %c,%b", "DIV0 %c,%i", "DIV %c,%a,%b", "DIV %c,%a,%i", "DIVL %c,%a,%b", "DIVL %c,%a,%i", "DIVREM %c,%a,%b", "DIVREM %c,%a,%i",
        "ASEQ %t,%a,%b", "ASEQ %t,%a,%i", "ASNEQ %t,%a,%b", "ASNEQ %t,%a,%i", "MULU %c,%a,%b", "MULU %c,%a,%i", "???", "???",
        "INHW %c,%a,%b", "INHW %c,%a,%i", "EXTRACT %c,%a,%b", "EXTRACT %c,%a,%i", "EXHW %c,%a,%b", "EXHW %c,%a,%i", "EXHWS %c,%a", "???",
        "SLL %c,%a,%b", "SLL %c,%a,%i", "SRL %c,%a,%b", "SRL %c,%a,%i", "???", "???", "SRA %c,%a,%b", "SRA %c,%a,%i",
        "IRET", "HALT", "???", "???", "IRETINV", "???", "???", "???",
        "AND %c,%a,%b", "AND %c,%a,%i", "OR %c,%a,%b", "OR %c,%a,%i", "XOR %c,%a,%b", "XOR %c,%a,%i", "XNOR %c,%a,%b", "XNOR %c,%a,%i",
        "NOR %c,%a,%b", "NOR %c,%a,%i", "NAND %c,%a,%b", "NAND %c,%a,%i", "ANDN %c,%a,%b", "ANDN %c,%a,%i", "SETIP %c,%a,%b", "INV",
        "JMP %r", "JMP %u", "???", "???", "JMPF %a,%r", "JMPF %a,%u", "???", "???",
        "CALL %a,%r", "CALL %a,%u", "ORN %c,%a,%b", "ORN %c,%a,%i", "JMPT %a,%r", "JMPT %a,%u", "???", "???",
        "???", "???", "???", "???", "JMPFDEC %a,%r", "JMPFDEC %a,%u", "MFTLB", "???",
        "???", "???", "???", "???", "???", "???", "MTTLB", "???",
        "JMPI %b", "???", "???", "???", "JMPFI %a,%b", "???", "MFSR %c,%s", "???",
        "CALLI %a,%b", "???", "???", "???", "JMPTI %a,%b", "???", "MTSR %s,%b", "???",
        "???", "???", "???", "???", "???", "???", "???", "EMULATE %c,%a,%b",
        "FMAC", "DMAC", "FMSM", "DMSM", "???", "???", "MULTM %c,%a,%b", "MULTMU %c,%a,%b",
        "MULTIPLY %c,%a,%b", "DIVIDE %c,%a,%b", "MULTIPLU %c,%a,%b", "DIVIDU %c,%a,%b", "CONVERT %c,%a,%f", "SQRT", "CLASS", "???",
        "MTACC", "MFACC", "FEQ %c,%a,%b", "DEQ %c,%a,%b", "FGT %c,%a,%b", "DGT %c,%a,%b", "FLT %c,%a,%b", "DLT %c,%a,%b",
        "FADD %c,%a,%b", "DADD %c,%a,%b", "FSUB %c,%a,%b", "DSUB %c,%a,%b", "FMUL %c,%a,%b", "DMUL %c,%a,%b", "FDIV %c,%a,%b", "DDIV %c,%a,%b",
        "???", "FDMUL", "???", "???", "nanochess_emulator %v,%b", "???", "???", "???",
    };
    char *q;
    int reg;
    uint32_t addr;
    
    /*
     ** The Am29000 doesn't have an instruction named NOP, but
     ** this is suggested by the AMD manuals.
     */
    if ((instruction >> 24) == 0x70) {  /* ASEQ */
        if (((instruction >> 8) & 0xff) == (instruction & 0xff)) {  /* Both registers are the same */
            strcpy(p, "NOP");
            return;
        }
    }
    q = mnemonic[instruction >> 24];    /* Get decoding information */
    while (*q) {
        if (*q == '%') {
            q++;
            switch (*q++) {
                case 'c':   /* Reg. C */
                    reg = (instruction >> 16) & 0xff;
                    p += sprintf(p, "%cr%d", (reg < 128 ? 'g' : 'l'), reg & 127);
                    break;
                case 'a':   /* Reg. A */
                    reg = (instruction >> 8) & 0xff;
                    p += sprintf(p, "%cr%d", (reg < 128 ? 'g' : 'l'), reg & 127);
                    break;
                case 'b':   /* Reg. B */
                    reg = (instruction & 0xff);
                    p += sprintf(p, "%cr%d", (reg < 128 ? 'g' : 'l'), reg & 127);
                    break;
                case 'l':   /* LOAD/STORE memory control */
                    p += sprintf(p, "%d,0x%02x", (instruction >> 23) & 1, (instruction >> 16) & 0x7f);
                    break;
                case 'i':   /* 8-bit immediate */
                    p += sprintf(p, "0x%02x", instruction & 0xff);
                    break;
                case 's':   /* Special register */
                    strcpy(p, special_regs[(instruction >> 8) & 0xff]);
                    while (*p)
                        p++;
                    break;
                case 'v':   /* Instruction code (emulator internal) */
                    p += sprintf(p, "0x%02x", (instruction >> 8) & 0xff);
                    break;
                case 't':   /* Trap 8-bit immediate */
                    p += sprintf(p, "0x%02x", (instruction >> 16) & 0xff);
                    break;
                case 'm':   /* 16-bit immediate */
                    p += sprintf(p, "0x%04x", (instruction & 0xff) | ((instruction >> 8) & 0xff00));
                    break;
                case 'r':   /* Relative address */
                    addr = ((instruction & 0xff) | ((instruction >> 8) & 0xff00)) << 2;
                    if (addr & 0x020000)
                        addr -= 0x040000;
                    addr += pc;
                    p += sprintf(p, "0x%08x", addr);
                    break;
                case 'u':   /* Absolute address */
                    addr = ((instruction & 0xff) | ((instruction >> 8) & 0xff00)) << 2;
                    p += sprintf(p, "0x%08x", addr);
                    break;
                case 'f':   /* CONVERT */
                    p += sprintf(p, "%x,%x,%x,%x", (instruction >> 7) & 1, (instruction >> 4) & 7, (instruction >> 2) & 3, instruction & 3);
                    break;
            }
        } else {
            *p++ = *q++;
        }
    }
/*    p += sprintf(p, "\tgr95=0x%08x", regs[95]);*/
    *p = '\0';
}

/*
 ** If you ask two times for the same hostname to Windows,
 ** it will stall you 15 seconds.
 */
#define MAX_DNS 32

struct dns_record {
    char hostname[256];
    uint32_t ip;
} dns[MAX_DNS];

/*
 ** Read DNS cache
 */
uint32_t dns_read_cache(char *hostname)
{
    struct dns_record temp;
    int c;
    int d;
    
    for (c = 0; c < MAX_DNS; c++) {
        if (strcmp(hostname, dns[c].hostname) == 0) {
            temp = dns[c];
            for (d = c - 1; d >= 0; d--)
                dns[d + 1] = dns[d];
            dns[0] = temp;  /* Move to front */
            return dns[0].ip;
        }
    }
    return 0;
}

/*
 ** Write DNS cache
 */
void dns_write_cache(char *hostname, uint32_t ip)
{
    int d;
    
    for (d = MAX_DNS - 2; d >= 0; d--)
        dns[d + 1] = dns[d];
    strcpy(dns[0].hostname, hostname); /* Put in front */
    dns[0].ip = ip;
}

#if AM29050
/*
 ** Floating-point support for Am29050
 */
float reg2float(uint32_t reg)
{
    union {
        uint32_t integer;
        float f;
    } v;
    v.integer = reg;
    return v.f;
}

uint32_t float2reg(float reg)
{
    union {
        uint32_t integer;
        float f;
    } v;
    v.f = reg;
    return v.integer;
}

double reg2double(uint32_t reg1, uint32_t reg2)
{
    union {
        uint32_t integer[2];
        double d;
    } v;
    v.integer[1] = reg1;    /* It will work only in little-endian machines */
    v.integer[0] = reg2;
    return v.d;
}

void double2reg(double reg, uint32_t *reg1, uint32_t *reg2)
{
    union {
        uint32_t integer[2];
        double d;
    } v;

    v.d = reg;
    *reg1 = v.integer[1];   /* It will work only in little-endian machines */
    *reg2 = v.integer[0];
}
#endif

/*
** Emulate one instruction
*/
void am29000_emulate(void)
{
    uint32_t instruction;
    uint32_t c;
    uint32_t d;
    int32_t e;
    int32_t f;
    uint64_t shift;
    uint8_t buffer[4];
    char string[256];
#if AM29050
    float fa, fb, fc;
    double da, db, dc;
#endif
    
    /* Detect free block number */
    /*            if (prev_value != read_word(0x8003b48c)) {
     prev_value = read_word(0x8003b48c);
     fprintf(stderr, "0x8000ff54 now is 0x%08x (cycle %d)\n", prev_value, count);
     }*/
    /* Timer */
    if ((special[8] & 0xffffff) == 0) { /* Timer reload */
        special[8] = special[9] & 0xffffff; /* Copy value */
        if (special[9] & 0x02000000)    /* Interrupt already happened? */
            special[9] |= 0x04000000;   /* Overflow */
        special[9] |= 0x02000000;   /* Interrupt */
    } else {
        special[8]--;    /* Count down */
    }
    if ((special[2] & 0x0401) == 0) { /* DA = 0 */
        if ((special[9] & 0x03000000) == 0x03000000) {  /* Interrupt + IE */
            special[10] = pc0;
            special[11] = pc1;
            special[12] = pc2;
            trap(14);
        }
    }
    
    instruction = read_word(pc1);   /* Read the next instruction to execute */
#if 0   /* Assembler debugging */
    {
        if (pc1 == 0x00050150) {    /* Writing data into the disk */
            fprintf(stderr, "Writing 0x%08x to 0x%08x (0x%08x bytes) ", regs[REG_AA(0x86)], regs[REG_AA(0x87)], regs[REG_AA(0x85)]);
            c = regs[REG_AA(0x86)];
            d = regs[REG_AA(0x85)];
            while (d--) {
                fprintf(stderr, "%02x ", read_byte(c));
                c++;
            }
            fprintf(stderr, "\n");
        }
        if (pc1 == 0x00050448) {
            fprintf(stderr, "Seeking (1) 0x%08x%08x\n", regs[REG_AA(0x88)], regs[REG_AA(0x89)]);
        }
        if (pc1 == 0x0005047c) {    /* Bug in ADDC found here */
            fprintf(stderr, "Seeking (2) 0x%08x%08x\n", regs[REG_AA(0x88)], regs[REG_AA(0x89)]);
        }
    }
#endif
#if 0
    {
        static int first_time = 1;
        static int program_base;
        static char buffer[64];
        static char *ap;
        
        if (first_time == 0) {
            if (pc1 == program_base + 6904) {   /* Entry for result (6512 for entry, 6712 after binary search ) */
                c = read_word(program_base + 6524);
                d = read_word(program_base + 6528);
                c = (c & 0xff) | ((c & 0xff0000) >> 8) | ((d & 0xff) << 16) | ((d & 0xff0000) << 8);
                ap = buffer;
                while (ap - buffer < 63) {
                    d = read_byte(c);
                    c++;
                    *ap++ = d;
                    if (d == 0)
                        break;
                }
                c = read_word(program_base + 6896);
                d = read_word(program_base + 6900);
                c = (c & 0xff) | ((c & 0xff0000) >> 8) | ((d & 0xff) << 16) | ((d & 0xff0000) << 8);
                fprintf(stderr, "busca_instruccion() = componente = %s (%d) (0x%08x)\n", buffer, regs[REG_AA(0x86)], read_word(c));
            }
        } else if (instruction == 0x14688485 && read_word(pc1 + 4) == 0x87866801) {
            
            if (first_time) {
                program_base = pc1 - 6616;
                first_time = 0;
            }
                
        }
    }
#endif
    
    if ((special[2] & 0x0400) == 0) {   /* Update PCs only if not in Freeze mode */
        special[10] = pc0;
        special[11] = pc1;
        special[12] = pc2;
    }
    count++;
    if (debug != NULL) {
        disassemble(pc1, instruction, string);
        fprintf(debug, "% 8d PC=0x%08X %s\n", count, pc1, string);
        /*            if (pc1 == 0x80032c08) {
         debug_info();
         exit(1);
         }*/
        /*            if (pc1 == 0x8000c7b8) {
         debug_info();
         exit(1);
         }*/
    }
    /*        if (pc0 == 0x8000c078) {     // Driver letter setup
     debug_info();
     exit(1);
     }*/
    pc2 = pc1;
    pc1 = pc0;
    pc0 = pc1 + 4;
    switch (instruction >> 24) {    /* Decode instruction */
        case 0x01:  /* CONSTN */
            REG_A = 0xffff0000 | IMM16;
            break;
        case 0x02:  /* CONSTH */
            REG_A = (REG_A & 0xffff) | (IMM16 << 16);
            break;
        case 0x03:  /* CONST */
            REG_A = IMM16;
            break;
        case 0x04:  /* MTSRIM */
            c = (instruction >> 8) & 0xff;
            if (c == 133) {
                WRITE_BP(IMM16);
            } else if (c == 134) {
                WRITE_FC(IMM16);
            } else {
                special[c] = IMM16;
                if (c == 3) {
                    if ((special[c] & 4) == 0) {    /* BO = 0 */
                        endianness = 3; /* Big-endian */
                    } else {    /* BO = 1 */
                        endianness = 0; /* Little-endian */
                    }
                }
            }
            /* !!! Add masks */
            break;
        case 0x05:  /* CONSTHZ */
            REG_A = IMM16 << 16;
            break;
        case 0x08:  /* CLZ */
            REG_C = clz(REG_B);
            break;
        case 0x09:  /* CLZ imm */
            REG_C = clz(IMM);
            break;
        case 0x0a:  /* EXBYTE */
            c = REG_B & ~0xff;
            d = READ_BP ^ endianness;
            if (d == 3) {
                c |= (REG_A >> 24) & 0xff;
            } else if (d == 2) {
                c |= (REG_A >> 16) & 0xff;
            } else if (d == 1) {
                c |= (REG_A >> 8) & 0xff;
            } else {
                c |= REG_A & 0xff;
            }
            REG_C = c;
            break;
        case 0x0b:  /* EXBYTE imm */
            c = IMM & ~0xff;
            d = READ_BP ^ endianness;
            if (d == 3) {
                c |= (REG_A >> 24) & 0xff;
            } else if (d == 2) {
                c |= (REG_A >> 16) & 0xff;
            } else if (d == 1) {
                c |= (REG_A >> 8) & 0xff;
            } else {
                c |= REG_A & 0xff;
            }
            REG_C = c;
            break;
        case 0x0c:  /* INBYTE */
            c = REG_B & 0xff;
            d = READ_BP ^ endianness;
            if (d == 3) {
                c = (REG_A & ~0xff000000) | (c << 24);
            } else if (d == 2) {
                c = (REG_A & ~0x00ff0000) | (c << 16);
            } else if (d == 1) {
                c = (REG_A & ~0x0000ff00) | (c << 8);
            } else {
                c = (REG_A & ~0x000000ff) | c;
            }
            REG_C = c;
            break;
        case 0x0d:  /* INBYTE imm */
            c = IMM;
            d = READ_BP ^ endianness;
            if (d == 3) {
                c = (REG_A & ~0xff000000) | (c << 24);
            } else if (d == 2) {
                c = (REG_A & ~0x00ff0000) | (c << 16);
            } else if (d == 1) {
                c = (REG_A & ~0x0000ff00) | (c << 8);
            } else {
                c = (REG_A & ~0x000000ff) | c;
            }
            REG_C = c;
            break;
        case 0x10:    /* ADDS */
            ALU(REG_A, REG_B, 0);
            REG_C = REG_A + REG_B;
            /* !!! Should cause a trap in case of overflow */
            break;
        case 0x11:    /* ADDS imm */
            ALU(REG_A, IMM, 0);
            REG_C = REG_A + IMM;
            /* !!! Should cause a trap in case of overflow */
            break;
        case 0x12:    /* ADDU */
            ALU(REG_A, REG_B, 0);
            REG_C = REG_A + REG_B;
            /* !!! Should cause a trap in case of overflow */
            break;
        case 0x13:    /* ADDU imm */
            ALU(REG_A, IMM, 0);
            REG_C = REG_A + IMM;
            /* !!! Should cause a trap in case of overflow */
            break;
        case 0x14:    /* ADD */
            ALU(REG_A, REG_B, 0);
            REG_C = REG_A + REG_B;
            break;
        case 0x15:    /* ADD imm */
            ALU(REG_A, IMM, 0);
            REG_C = REG_A + IMM;
            break;
        case 0x16:  /* LOAD */
            if ((instruction & 0x00100000) != 0) {  /* Set byte pointer */
                c = REG_B & 3;
                WRITE_BP(c);
            }
            switch ((instruction >> 16) & 0xef) {
                case 0x00:
                case 0x20:
                    if (mode == 1) {
                        REG_A = clgd5440_pci_mem_read_dword(REG_B);
                    } else {
                        fprintf(stderr, "Unhandled memory control 0x%08x\n", instruction);
                        debug_info();
                        exit(1);
                    }
                    break;
                case 0x01:
                    if (mode == 1) {
                        REG_A = clgd5440_pci_mem_read_dword(REG_B);
                    } else {
                        fprintf(stderr, "Unhandled memory control 0x%08x\n", instruction);
                        debug_info();
                        exit(1);
                    }
                    break;
                case 0x02:
                    if (mode == 1) {
                        REG_A = clgd5440_pci_mem_read_word(REG_B);
                    } else if (mode == 0) {
                        REG_A = clgd5429_mem_read_word(REG_B / 4);
                    } else {
                        fprintf(stderr, "Unhandled memory control 0x%08x\n", instruction);
                        debug_info();
                        exit(1);
                    }
                    break;
                case 0x04:
                    REG_A = read_word(REG_B);
                    break;
                case 0x41:
                case 0x61:
                    REG_A = read_isa(REG_B);
                    break;
                default:
                    fprintf(stderr, "Unhandled memory control 0x%08x\n", instruction);
                    debug_info();
                    exit(1);
            }
            break;
        case 0x17:  /* LOAD imm */
            if ((instruction & 0x00100000) != 0) {  /* Set byte pointer */
                c = REG_B & 3;
                WRITE_BP(c);
            }
            switch ((instruction >> 16) & 0xef) {
                case 0x04:
                    REG_A = read_word(IMM);
                    break;
                case 0x41:
                case 0x61:
                    REG_A = read_isa(IMM);
                    break;
                default:
                    fprintf(stderr, "Unhandled memory control 0x%08x\n", instruction);
                    debug_info();
                    exit(1);
            }
            break;
        case 0x18:    /* ADDCS */
            c = ALU_CARRY;
            ALU(REG_A, REG_B, c);
            REG_C = REG_A + REG_B + c;
            /* !!! Should cause a trap in case of overflow */
            break;
        case 0x19:    /* ADDCS imm */
            c = ALU_CARRY;
            ALU(REG_A, IMM, c);
            REG_C = REG_A + IMM + c;
            /* !!! Should cause a trap in case of overflow */
            break;
        case 0x1a:    /* ADDCU */
            c = ALU_CARRY;
            ALU(REG_A, REG_B, c);
            REG_C = REG_A + REG_B + c;
            /* !!! Should cause a trap in case of overflow */
            break;
        case 0x1b:    /* ADDCU imm */
            c = ALU_CARRY;
            ALU(REG_A, IMM, c);
            REG_C = REG_A + IMM + c;
            /* !!! Should cause a trap in case of overflow */
            break;
        case 0x1c:    /* ADDC */
            c = ALU_CARRY;
            ALU(REG_A, REG_B, c);
            REG_C = REG_A + REG_B + c;
            break;
        case 0x1d:    /* ADDC imm */
            c = ALU_CARRY;
            ALU(REG_A, IMM, c);
            REG_C = REG_A + IMM + c;
            break;
        case 0x1e:  /* STORE */
            if ((instruction & 0x00100000) != 0) {  /* Set byte pointer */
                c = REG_B & 3;
                WRITE_BP(c);
            }
            switch ((instruction >> 16) & 0xef) {
                case 0x00:
                case 0x20:
                    c = REG_B;
                    d = REG_A;
                    if (mode == 1)
                        e = clgd5440_pci_mem_write_dword(c, d);
                    else
                        e = clgd5429_mem_write_byte(c / 4, d & 0xff);
                    if (e == -1) {
                        fprintf(stderr, "Unhandled memory control 0x%08x\n", instruction);
                        debug_info();
                        exit(1);
                    }
                    break;
                case 0x01:
                    c = REG_B;
                    d = REG_A;
                    if (mode == 1)
                        e = clgd5440_pci_mem_write_byte(c, d);
                    else
                        e = -1;
                    if (e == -1) {
                        fprintf(stderr, "Unhandled memory control 0x%08x\n", instruction);
                        debug_info();
                        exit(1);
                    }
                    break;
                case 0x02:
                    c = REG_B;
                    d = REG_A;
                    if (mode == 1)
                        e = clgd5440_pci_mem_write_word(c, d);
                    else
                        e = clgd5429_mem_write_word(c / 4, d & 0xffff);
                    if (e == -1) {
                        fprintf(stderr, "Unhandled memory control 0x%08x\n", instruction);
                        debug_info();
                        exit(1);
                    }
                    break;
                case 0x04:
                    /*                    if (REG_B == 0x80037ca8) {
                     fprintf(stderr, "Watching address 0x%08x now written with 0x%08x\n", REG_B, REG_A);
                     if (REG_A == 0xffffffffU) {
                     debug_info();
                     exit(1);
                     }
                     }*/
                    write_word(REG_B, REG_A);
                    break;
                case 0x41:
                case 0x61:
                    write_isa(REG_B, REG_A);
                    break;
                case 0x42:
                case 0x62:
                    write_isaw(REG_B, REG_A);
                    break;
                default:
                    fprintf(stderr, "Unhandled memory control 0x%08x\n", instruction);
                    debug_info();
                    exit(1);
            }
            break;
        case 0x1f:  /* STORE */
            if ((instruction & 0x00100000) != 0) {  /* Set byte pointer */
                c = REG_B & 3;
                WRITE_BP(c);
            }
            switch ((instruction >> 16) & 0xef) {
                case 0x04:
                    write_word(IMM, REG_A);
                    break;
                case 0x41:
                case 0x61:
                    write_isa(IMM, REG_A);
                    break;
                default:
                    fprintf(stderr, "Unhandled memory control 0x%08x\n", instruction);
                    debug_info();
                    exit(1);
            }
            break;
        case 0x20:    /* SUBS */
            ALU(REG_A, -REG_B, 0);
            REG_C = REG_A - REG_B;
            /* !!! Generate trap if signed value overflows */
            break;
        case 0x21:    /* SUBS imm */
            ALU(REG_A, -IMM, 0);
            REG_C = REG_A - IMM;
            /* !!! Generate trap if signed value overflows */
            break;
        case 0x22:    /* SUBU */
            ALU(REG_A, -REG_B, 0);
            REG_C = REG_A - REG_B;
            /* !!! Generate trap if unsigned value overflows */
            break;
        case 0x23:    /* SUBU imm */
            ALU(REG_A, -IMM, 0);
            REG_C = REG_A - IMM;
            /* !!! Generate trap if unsigned value overflows */
            break;
        case 0x24:    /* SUB */
            ALU(REG_A, -REG_B, 0);
            REG_C = REG_A - REG_B;
            break;
        case 0x25:    /* SUB imm */
            ALU(REG_A, -IMM, 0);
            REG_C = REG_A - IMM;
            break;
        case 0x28:    /* SUBCS */
            c = ALU_CARRY;
            ALU(REG_A, ~REG_B, c);
            REG_C = REG_A + ~REG_B + c;
            /* !!! Generate trap if signed value overflows */
            break;
        case 0x29:    /* SUBCS imm */
            c = ALU_CARRY;
            ALU(REG_A, ~IMM, c);
            REG_C = REG_A + ~IMM + c;
            /* !!! Generate trap if signed value overflows */
            break;
        case 0x2a:    /* SUBCU */
            c = ALU_CARRY;
            ALU(REG_A, ~REG_B, c);
            REG_C = REG_A + ~REG_B + c;
            /* !!! Generate trap if unsigned value overflows */
            break;
        case 0x2b:    /* SUBCU imm */
            c = ALU_CARRY;
            ALU(REG_A, ~IMM, c);
            REG_C = REG_A + ~IMM + c;
            /* !!! Generate trap if unsigned value overflows */
            break;
        case 0x2c:    /* SUBC */
            c = ALU_CARRY;
            ALU(REG_A, ~REG_B, c);
            REG_C = REG_A + ~REG_B + c;
            break;
        case 0x2d:    /* SUBC imm */
            c = ALU_CARRY;
            ALU(REG_A, ~IMM, c);
            REG_C = REG_A + ~IMM + c;
            break;
        case 0x2e:    /* CPBYTE */
            c = REG_A ^ REG_B;
            if ((c & 0xff) == 0 || (c & 0xff00) == 0 || (c & 0xff0000) == 0 || (c & 0xff000000) == 0) {
                REG_C = AM29K_TRUE;
            } else {
                REG_C = AM29K_FALSE;
            }
            break;
        case 0x2f:    /* CPBYTE imm */
            c = REG_A ^ IMM;
            if ((c & 0xff) == 0 || (c & 0xff00) == 0 || (c & 0xff0000) == 0 || (c & 0xff000000) == 0) {
                REG_C = AM29K_TRUE;
            } else {
                REG_C = AM29K_FALSE;
            }
            break;
        case 0x30:    /* SUBRS */
            ALU(-REG_A, REG_B, 0);
            REG_C = REG_B - REG_A;
            /* !!! Generate trap if signed value overflows */
            break;
        case 0x31:    /* SUBRS imm */
            ALU(-REG_A, IMM, 0);
            REG_C = IMM - REG_A;
            /* !!! Generate trap if signed value overflows */
            break;
        case 0x32:    /* SUBRU */
            ALU(-REG_A, REG_B, 0);
            /* !!! Generate trap if unsigned value overflows */
            REG_C = REG_B - REG_A;
            break;
        case 0x33:    /* SUBRU imm */
            ALU(-REG_A, IMM, 0);
            REG_C = IMM - REG_A;
            /* !!! Generate trap if unsigned value overflows */
            break;
        case 0x34:    /* SUBR */
            ALU(-REG_A, REG_B, 0);
            REG_C = REG_B - REG_A;
            break;
        case 0x35:    /* SUBR imm */
            ALU(-REG_A, IMM, 0);
            REG_C = IMM - REG_A;
            break;
        case 0x36:  /* LOADM */
            switch ((instruction >> 16) & 0xff) {
                case 0x04:
                    c = REG_B;
                    d = _RA;
                    if (d == 0)
                        d = (special[129] >> 2) & 0xff;
                    else if (d >= 128)
                        d = ((regs[1] / 4 + d) & 0x7f) | 0x80;
                    while (1) {
                        regs[d] = read_word(c);
                        if ((special[135] & 0xff) == 0)
                            break;
                        c += 4;
                        --special[135];
                        ++d;
                        if (d == 128)
                            d = ((regs[1] / 4) & 0x7f) | 0x80;
                        else if (d == 256)
                            d = 0x80;
                    }
                    break;
                default:
                    fprintf(stderr, "Unhandled memory control 0x%08x\n", instruction);
                    debug_info();
                    exit(1);
            }
            break;
        case 0x38:    /* SUBRCS */
            c = ALU_CARRY;
            ALU(~REG_A, REG_B, c);
            REG_C = ~REG_A + REG_B + c;
            /* !!! Generate trap if signed value overflows */
            break;
        case 0x39:    /* SUBRCS imm */
            c = ALU_CARRY;
            ALU(~REG_A, IMM, c);
            REG_C = ~REG_A + IMM + c;
            /* !!! Generate trap if signed value overflows */
            break;
        case 0x3a:    /* SUBRCU */
            c = ALU_CARRY;
            ALU(~REG_A, REG_B, c);
            REG_C = ~REG_A + REG_B + c;
            /* !!! Generate trap if unsigned value overflows */
            break;
        case 0x3b:    /* SUBRCU imm */
            c = ALU_CARRY;
            ALU(~REG_A, IMM, c);
            REG_C = ~REG_A + IMM + c;
            /* !!! Generate trap if unsigned value overflows */
            break;
        case 0x3c:    /* SUBRC */
            c = ALU_CARRY;
            ALU(~REG_A, REG_B, c);
            REG_C = ~REG_A + REG_B + c;
            break;
        case 0x3d:    /* SUBRC imm */
            c = ALU_CARRY;
            ALU(~REG_A, IMM, c);
            REG_C = ~REG_A + IMM + c;
            break;
        case 0x3e:  /* STOREM */
            switch ((instruction >> 16) & 0xff) {
                case 0x04:
                    c = REG_B;
                    d = _RA;
                    if (d == 0)
                        d = (special[129] >> 2) & 0xff;
                    else if (d >= 128)
                        d = ((regs[1] / 4 + d) & 0x7f) | 0x80;
                    while (1) {
                        write_word(c, regs[d]);
                        if ((special[135] & 0xff) == 0)
                            break;
                        c += 4;
                        --special[135];
                        ++d;
                        if (d == 128)
                            d = ((regs[1] / 4) & 0x7f) | 0x80;
                        else if (d == 256)
                            d = 0x80;
                    }
                    break;
                default:
                    fprintf(stderr, "Unhandled memory control 0x%08x\n", instruction);
                    debug_info();
                    exit(1);
            }
            break;
        case 0x40:    /* CPLT */
            e = REG_A;
            f = REG_B;
            REG_C = (e < f) ? AM29K_TRUE : AM29K_FALSE;
            break;
        case 0x41:    /* CPLT imm */
            e = REG_A;
            f = IMM;
            REG_C = (e < f) ? AM29K_TRUE : AM29K_FALSE;
            break;
        case 0x42:    /* CPLTU */
            c = REG_A;
            d = REG_B;
            REG_C = (c < d) ? AM29K_TRUE : AM29K_FALSE;
            break;
        case 0x43:    /* CPLTU imm */
            c = REG_A;
            d = IMM;
            REG_C = (c < d) ? AM29K_TRUE : AM29K_FALSE;
            break;
        case 0x44:    /* CPLE */
            e = REG_A;
            f = REG_B;
            REG_C = (e <= f) ? AM29K_TRUE : AM29K_FALSE;
            break;
        case 0x45:    /* CPLE imm */
            e = REG_A;
            f = IMM;
            REG_C = (e <= f) ? AM29K_TRUE : AM29K_FALSE;
            break;
        case 0x46:    /* CPLEU */
            c = REG_A;
            d = REG_B;
            REG_C = (c <= d) ? AM29K_TRUE : AM29K_FALSE;
            break;
        case 0x47:    /* CPLEU imm */
            c = REG_A;
            d = IMM;
            REG_C = (c <= d) ? AM29K_TRUE : AM29K_FALSE;
            break;
        case 0x48:    /* CPGT */
            e = REG_A;
            f = REG_B;
            REG_C = (e > f) ? AM29K_TRUE : AM29K_FALSE;
            break;
        case 0x49:    /* CPGT imm */
            e = REG_A;
            f = IMM;
            REG_C = (e > f) ? AM29K_TRUE : AM29K_FALSE;
            break;
        case 0x4a:    /* CPGTU */
            c = REG_A;
            d = REG_B;
            REG_C = (c > d) ? AM29K_TRUE : AM29K_FALSE;
            break;
        case 0x4b:    /* CPGTU imm */
            c = REG_A;
            d = IMM;
            REG_C = (c > d) ? AM29K_TRUE : AM29K_FALSE;
            break;
        case 0x4c:    /* CPGE */
            e = REG_A;
            f = REG_B;
            REG_C = (e >= f) ? AM29K_TRUE : AM29K_FALSE;
            break;
        case 0x4d:    /* CPGE imm */
            e = REG_A;
            f = IMM;
            REG_C = (e >= f) ? AM29K_TRUE : AM29K_FALSE;
            break;
        case 0x4e:    /* CPGEU */
            c = REG_A;
            d = REG_B;
            REG_C = (c >= d) ? AM29K_TRUE : AM29K_FALSE;
            break;
        case 0x4f:    /* CPGEU imm */
            c = REG_A;
            d = IMM;
            REG_C = (c >= d) ? AM29K_TRUE : AM29K_FALSE;
            break;
        case 0x50:    /* ASLT */
            e = REG_A;
            f = REG_B;
            if (e < f)
                ;
            else
                trap((instruction >> 16) & 0xff);
            break;
        case 0x51:    /* ASLT imm */
            e = REG_A;
            f = IMM;
            if (e < f)
                ;
            else
                trap((instruction >> 16) & 0xff);
            break;
        case 0x52:    /* ASLTU */
            c = REG_A;
            d = REG_B;
            if (c < d)
                ;
            else
                trap((instruction >> 16) & 0xff);
            break;
        case 0x53:    /* ASLTU imm */
            c = REG_A;
            d = IMM;
            if (c < d)
                ;
            else
                trap((instruction >> 16) & 0xff);
            break;
        case 0x54:    /* ASLE */
            e = REG_A;
            f = REG_B;
            if (e <= f)
                ;
            else
                trap((instruction >> 16) & 0xff);
            break;
        case 0x55:    /* ASLE imm */
            e = REG_A;
            f = IMM;
            if (e <= f)
                ;
            else
                trap((instruction >> 16) & 0xff);
            break;
        case 0x56:    /* ASLEU */
            c = REG_A;
            d = REG_B;
            if (c <= d)
                ;
            else
                trap((instruction >> 16) & 0xff);
            break;
        case 0x57:    /* ASLEU imm */
            c = REG_A;
            d = IMM;
            if (c <= d)
                ;
            else
                trap((instruction >> 16) & 0xff);
            break;
        case 0x58:    /* ASGT */
            e = REG_A;
            f = REG_B;
            if (e > f)
                ;
            else
                trap((instruction >> 16) & 0xff);
            break;
        case 0x59:    /* ASGT imm */
            e = REG_A;
            f = IMM;
            if (e > f)
                ;
            else
                trap((instruction >> 16) & 0xff);
            break;
        case 0x5a:    /* ASGTU */
            c = REG_A;
            d = REG_B;
            if (c >= d)
                ;
            else
                trap((instruction >> 16) & 0xff);
            break;
        case 0x5b:    /* ASGTU imm */
            c = REG_A;
            d = IMM;
            if (c >= d)
                ;
            else
                trap((instruction >> 16) & 0xff);
            break;
        case 0x5c:    /* ASGE */
            e = REG_A;
            f = REG_B;
            if (e >= f)
                ;
            else
                trap((instruction >> 16) & 0xff);
            break;
        case 0x5d:    /* ASGE imm */
            e = REG_A;
            f = IMM;
            if (e >= f)
                ;
            else
                trap((instruction >> 16) & 0xff);
            break;
        case 0x5e:    /* ASGEU */
            c = REG_A;
            d = REG_B;
            if (c >= d)
                ;
            else
                trap((instruction >> 16) & 0xff);
            break;
        case 0x5f:    /* ASGEU imm */
            c = REG_A;
            d = IMM;
            if (c >= d)
                ;
            else
                trap((instruction >> 16) & 0xff);
            break;
        case 0x60:    /* CPEQ */
            c = REG_A;
            d = REG_B;
            REG_C = (c == d) ? AM29K_TRUE : AM29K_FALSE;
            break;
        case 0x61:    /* CPEQ imm */
            c = REG_A;
            d = IMM;
            REG_C = (c == d) ? AM29K_TRUE : AM29K_FALSE;
            break;
        case 0x62:    /* CPNEQ */
            c = REG_A;
            d = REG_B;
            REG_C = (c != d) ? AM29K_TRUE : AM29K_FALSE;
            break;
        case 0x63:    /* CPNEQ imm */
            c = REG_A;
            d = IMM;
            REG_C = (c != d) ? AM29K_TRUE : AM29K_FALSE;
            break;
        case 0x64:  /* MUL */
            c = REG_B;
            d = (special[131] & 1) ? REG_A : 0;
            ALU(c, d, 0);
            e = c + d;
            d = e & 0x80000000;
            if (((c ^ d) & 0x80000000) == 0 && ((c ^ e) & 0x80000000) != 0) {
                d ^= 0x80000000;
            }
            special[131] = (special[131] >> 1) | (e << 31);
            c = (e >> 1) | d;
            REG_C = c;
            break;
        case 0x65:  /* MUL imm */
            c = IMM;
            d = (special[131] & 1) ? REG_A : 0;
            ALU(c, d, 0);
            e = c + d;
            d = e & 0x80000000;
            if (((c ^ d) & 0x80000000) == 0 && ((c ^ e) & 0x80000000) != 0) {
                d ^= 0x80000000;
            }
            special[131] = (special[131] >> 1) | (e << 31);
            c = (e >> 1) | d;
            REG_C = c;
            break;
        case 0x66:  /* MULL */
            c = REG_B;
            d = (special[131] & 1) ? REG_A : 0;
            ALU(c, 0 - d, 0);
            e = c - d;
            d = e & 0x80000000;
            if ((c ^ d) & (c ^ e) & 0x80000000) {
                d ^= 0x80000000;
            }
            special[131] = (special[131] >> 1) | (e << 31);
            c = (e >> 1) | d;
            REG_C = c;
            break;
        case 0x67:  /* MULL imm */
            c = IMM;
            d = (special[131] & 1) ? REG_A : 0;
            ALU(c, 0 - d, 0);
            e = c - d;
            d = e & 0x80000000;
            if ((c ^ d) & (c ^ e) & 0x80000000) {
                d ^= 0x80000000;
            }
            special[131] = (special[131] >> 1) | (e << 31);
            c = (e >> 1) | d;
            REG_C = c;
            break;
        case 0x68:  /* DIV0 */
            special[132] |= 0x0800; /* DF */
            c = REG_B;
            if (c & 0x80000000)
                special[132] |= 0x0200;
            else
                special[132] &= ~0x0200;
            c = (c << 1) | (special[131] >> 31);
            special[131] <<= 1;
            REG_C = c;
            break;
        case 0x69:  /* DIV0 imm */
            special[132] |= 0x0800; /* DF */
            c = IMM;
            if (c & 0x80000000)
                special[132] |= 0x0200;
            else
                special[132] &= ~0x0200;
            c = (c << 1) | (special[131] >> 31);
            special[131] <<= 1;
            REG_C = c;
            break;
        case 0x6a:  /* DIV */
            c = REG_A;
            d = REG_B;
            if (special[132] & 0x0800) {
                if (c < d)
                    e = 1;
                else
                    e = 0;
                c = c - d;
            } else {
                if (c + d < c)
                    e = 0;
                else
                    e = 1;
                c = c + d;
            }
            d = ((special[132] >> 11) ^ (special[132] >> 9) ^ e) & 1;
            special[132] = (special[132] & ~0x0a00) | (d << 11) | ((c & 0x80000000) >> 22);
            c = (c << 1) | (special[131] >> 31);
            special[131] = (special[131] << 1) | d;
            REG_C = c;
            break;
        case 0x6b:  /* DIV imm */
            c = REG_A;
            d = IMM;
            if (special[132] & 0x0800) {
                if (c < d)
                    e = 1;
                else
                    e = 0;
                c = c - d;
            } else {
                if (c + d < c)
                    e = 0;
                else
                    e = 1;
                c = c + d;
            }
            d = ((special[132] >> 11) ^ (special[132] >> 9) ^ e) & 1;
            special[132] = (special[132] & ~0x0a00) | (d << 11) | ((c & 0x80000000) >> 22);
            c = (c << 1) | (special[131] >> 31);
            special[131] = (special[131] << 1) | d;
            REG_C = c;
            break;
        case 0x6c:  /* DIVL */
            c = REG_A;
            d = REG_B;
            if (special[132] & 0x0800) {
                if (c < d)
                    e = 1;
                else
                    e = 0;
                c = c - d;
            } else {
                if (c + d < c)
                    e = 0;
                else
                    e = 1;
                c = c + d;
            }
            d = ((special[132] >> 11) ^ (special[132] >> 9) ^ e) & 1;
            special[132] = (special[132] & ~0x0a00) | (d << 11) | ((c & 0x80000000) >> 22);
            special[131] = (special[131] << 1) | d;
            REG_C = c;
            break;
        case 0x6d:  /* DIVL imm */
            c = REG_A;
            d = IMM;
            if (special[132] & 0x0800) {
                if (c < d)
                    e = 1;
                else
                    e = 0;
                c = c - d;
            } else {
                if (c + d < c)
                    e = 0;
                else
                    e = 1;
                c = c + d;
            }
            d = ((special[132] >> 11) ^ (special[132] >> 9) ^ e) & 1;
            special[132] = (special[132] & ~0x0a00) | (d << 11) | ((c & 0x80000000) >> 22);
            special[131] = (special[131] << 1) | d;
            REG_C = c;
            break;
        case 0x6e:  /* DIVREM */
            if (special[132] & 0x0800) {
                c = REG_A;
            } else {
                ALU(REG_A, REG_B, 0);
                c = REG_A + REG_B;
            }
            REG_C = c;
            break;
        case 0x6f:  /* DIVREM imm */
            if (special[132] & 0x0800) {
                c = REG_A;
            } else {
                ALU(REG_A, IMM, 0);
                c = REG_A + IMM;
            }
            REG_C = c;
            break;
        case 0x70:    /* ASEQ */
            c = REG_A;
            d = REG_B;
            if (c == d)
                ;
            else
                trap((instruction >> 16) & 0xff);
            break;
        case 0x71:    /* ASEQ imm */
            c = REG_A;
            d = IMM;
            if (c == d)
                ;
            else
                trap((instruction >> 16) & 0xff);
            break;
        case 0x72:    /* ASNEQ */
            c = REG_A;
            d = REG_B;
            if (c != d)
                ;
            else
                trap((instruction >> 16) & 0xff);
            break;
        case 0x73:    /* ASNEQ imm */
            c = REG_A;
            d = IMM;
            if (c != d)
                ;
            else
                trap((instruction >> 16) & 0xff);
            break;
        case 0x74:  /* MULU */
            if (special[131] & 1) {
                ALU(REG_B, REG_A, 0);
                c = REG_B + REG_A;
            } else {
                ALU(REG_B, 0, 0);
                c = REG_B;
            }
            special[131] = (special[131] >> 1) | (c << 31);
            c = (c >> 1) | ((special[132] & 0x80) << 24);
            REG_C = c;
            break;
        case 0x75:  /* MULU imm */
            if (special[131] & 1) {
                ALU(IMM, REG_A, 0);
                c = IMM + REG_A;
            } else {
                ALU(IMM, 0, 0);
                c = IMM;
            }
            special[131] = (special[131] >> 1) | (c << 31);
            c = (c >> 1) | ((special[132] & 0x80) << 24);
            REG_C = c;
            break;
        case 0x78:  /* INHW */
            c = REG_B & 0xffff;
            d = READ_BP ^ endianness;
            if ((d & 2) == 0) {
                c = (REG_A & ~0x0000ffff) | c;
            } else {
                c = (REG_A & ~0xffff0000) | (c << 16);
            }
            REG_C = c;
            break;
        case 0x79:  /* INHW imm */
            c = IMM & 0xffff;
            d = READ_BP ^ endianness;
            if ((d & 2) == 0) {
                c = (REG_A & ~0x0000ffff) | c;
            } else {
                c = (REG_A & ~0xffff0000) | (c << 16);
            }
            REG_C = c;
            break;
        case 0x7a:  /* EXTRACT */
            /*                fprintf(stderr, "%08x %08x (%d) = ", REG_A, REG_B, special[134]);*/
            shift = (((uint64_t) REG_A) << 32) | REG_B;
            shift <<= READ_FC;
            REG_C = (uint32_t) (shift >> 32);
            /*                fprintf(stderr, "%08x\n", REG_C);*/
            break;
        case 0x7b:  /* EXTRACT imm */
            shift = (((uint64_t) REG_A) << 32) | IMM;
            shift <<= READ_FC;
            REG_C = (uint32_t) (shift >> 32);
            break;
        case 0x7c:  /* EXHW */
            c = REG_B & 0xffff0000;
            d = READ_BP ^ endianness;
            if ((d & 2) == 0) {
                d = REG_A;
            } else {
                d = REG_A >> 16;
            }
            REG_C = c | (d & 0xffff);
            break;
        case 0x7d:  /* EXHW imm */
            c = IMM & 0xffff0000;
            d = READ_BP ^ endianness;
            if ((d & 2) == 0) {
                d = REG_A;
            } else {
                d = REG_A >> 16;
            }
            REG_C = c | (d & 0xffff);
            break;
        case 0x7e:  /* EXHWS */
            d = READ_BP ^ endianness;
            if ((d & 2) == 0) {
                d = REG_A;
            } else {
                d = REG_A >> 16;
            }
            d &= 0xffff;
            if (d >= 0x8000)
                d -= 0x10000;
            REG_C = d;
            break;
        case 0x80:    /* SLL */
            c = REG_A << (REG_B & 0x1f);
            REG_C = c;
            break;
        case 0x81:    /* SLL imm */
            c = REG_A << (IMM & 0x1f);
            REG_C = c;
            break;
        case 0x82:    /* SRL */
            c = REG_A >> (REG_B & 0x1f);
            REG_C = c;
            break;
        case 0x83:    /* SRL imm */
            c = REG_A >> (IMM & 0x1f);
            REG_C = c;
            break;
        case 0x86:    /* SRA */
            e = REG_A;
            e >>= REG_B & 0x1f;
            REG_C = e;
            break;
        case 0x87:    /* SRA imm */
            e = REG_A;
            e >>= IMM & 0x1f;
            REG_C = e;
            break;
        case 0x88:  /* IRET */
            pc0 = special[10];
            pc1 = special[11];
            special[2] = special[1];
            break;
        case 0x89:  /* HALT */
            fprintf(stderr, "HALT detected.\n");
            exit(1);
            break;
        case 0x8c:  /* IRETINV */
            pc0 = special[10];
            pc1 = special[11];
            special[2] = special[1];
            break;
        case 0x90:    /* AND */
            c = REG_A & REG_B;
            REG_C = c;
            ALU_SIMPLE(c);
            break;
        case 0x91:    /* AND imm */
            c = REG_A & IMM;
            REG_C = c;
            ALU_SIMPLE(c);
            break;
        case 0x92:    /* OR */
            c = REG_A | REG_B;
            REG_C = c;
            ALU_SIMPLE(c);
            break;
        case 0x93:    /* OR imm */
            c = REG_A | IMM;
            REG_C = c;
            ALU_SIMPLE(c);
            break;
        case 0x94:    /* XOR */
            c = REG_A ^ REG_B;
            REG_C = c;
            ALU_SIMPLE(c);
            break;
        case 0x95:    /* XOR imm */
            c = REG_A ^ IMM;
            REG_C = c;
            ALU_SIMPLE(c);
            break;
        case 0x96:    /* XNOR */
            c = ~(REG_A ^ REG_B);
            REG_C = c;
            ALU_SIMPLE(c);
            break;
        case 0x97:    /* XNOR imm */
            c = ~(REG_A ^ IMM);
            REG_C = c;
            ALU_SIMPLE(c);
            break;
        case 0x98:    /* NOR */
            c = ~(REG_A | REG_B);
            REG_C = c;
            ALU_SIMPLE(c);
            break;
        case 0x99:    /* NOR imm */
            c = ~(REG_A | IMM);
            REG_C = c;
            ALU_SIMPLE(c);
            break;
        case 0x9a:    /* NAND */
            c = ~(REG_A & REG_B);
            REG_C = c;
            ALU_SIMPLE(c);
            break;
        case 0x9b:    /* NAND imm */
            c = ~(REG_A & IMM);
            REG_C = c;
            ALU_SIMPLE(c);
            break;
        case 0x9c:    /* ANDN */
            c = REG_A & ~REG_B;
            REG_C = c;
            ALU_SIMPLE(c);
            break;
        case 0x9d:    /* ANDN imm */
            c = REG_A & ~IMM;
            REG_C = c;
            ALU_SIMPLE(c);
            break;
        case 0x9e:  /* SETIP */
            special[128] = REG_CA(_RC) * 4;
            special[129] = REG_AA(_RA) * 4;
            special[130] = REG_BA(_RB) * 4;
            break;
        case 0xa0:  /* JMP rel */
            pc0 = pc2 + (IMM16S << 2);
            break;
        case 0xa1:  /* JMP abs */
            pc0 = IMM16 << 2;
            break;
        case 0xa4:  /* JMPF rel */
            if ((REG_A & 0x80000000) == 0) {
                pc0 = pc2 + (IMM16S << 2);
            }
            break;
        case 0xa5:  /* JMPF abs */
            if ((REG_A & 0x80000000) == 0) {
                pc0 = IMM16 << 2;
            }
            break;
        case 0xa8:  /* CALL rel */
            pc0 = pc2 + (IMM16S << 2);
            REG_A = pc2 + 8;
            break;
        case 0xa9:  /* CALL abs */
            pc0 = IMM16 << 2;
            REG_A = pc2 + 8;
            break;
        case 0xaa:    /* ORN */
            c = REG_A | ~REG_B;
            REG_C = c;
            ALU_SIMPLE(c);
            break;
        case 0xab:    /* ORN imm */
            c = REG_A | ~IMM;
            REG_C = c;
            ALU_SIMPLE(c);
            break;
        case 0xac:  /* JMPT rel */
            if ((REG_A & 0x80000000) != 0) {
                pc0 = pc2 + (IMM16S << 2);
            }
            break;
        case 0xad:  /* JMPT abs */
            if ((REG_A & 0x80000000) != 0) {
                pc0 = IMM16 << 2;
            }
            break;
        case 0xb4:  /* JMPFDEC rel */
            c = REG_A;
            REG_A = c - 1;
            if ((c & 0x80000000) == 0) {
                pc0 = pc2 + (IMM16S << 2);
            }
            break;
        case 0xb5:  /* JMPFDEC abs */
            c = REG_A;
            REG_A = c - 1;
            if ((c & 0x80000000) == 0) {
                pc0 = IMM16 << 2;
            }
            break;
        case 0xc0:  /* JMPI */
            pc0 = REG_B;
            break;
        case 0xc4:  /* JMPFI */
            if ((REG_A & 0x80000000) == 0) {
                pc0 = REG_B;
            }
            break;
        case 0xc6:  /* MFSR */
            c = (instruction >> 8) & 0xff;
            if (c == 133)   /* BP */
                c = READ_BP;
            else if (c == 134)   /* FC */
                c = READ_FC;
            else
                c = special[c];
            REG_C = c;
            break;
        case 0xc8:  /* CALLI */
            pc0 = REG_B;
            REG_A = pc2 + 8;
            break;
        case 0xcc:  /* JMPTI */
            if ((REG_A & 0x80000000) != 0) {
                pc0 = REG_B;
            }
            break;
        case 0xce:  /* MTSR */
            c = (instruction >> 8) & 0xff;
            if (c == 133) {
                WRITE_BP(REG_B);
            } else if (c == 134) {
                WRITE_FC(REG_B);
            } else {
                special[c] = REG_B;
                if (c == 3) {
                    if ((special[c] & 4) == 0) {    /* BO = 0 */
                        endianness = 3; /* Big-endian */
                    } else {    /* BO = 1 */
                        endianness = 0; /* Little-endian */
                    }
                }
            }
            if (c == 0) {   /* VAB */
                static int first_time = 1;
                
                if (first_time) {
                    first_time = 0;
                    
                    if (mode == 0) {    /* G11V1 */
                        
                        /*
                         ** Patch the floppy disk code
                         */
                        write_word(0x8002086c, 0x4e618260); /* Support two drives */
                        write_word(0x80020878, 0xfc000280); /* Call the emulator */
                        
                        /*
                         ** Patch a math emulator strange error
                         ** It makes it to crash with CONVERT gr96,lr4,0,0,2,1 because fraction is non-zero
                         ** Probably the Am29000 was replaced with an Am29050 and I inserted
                         ** more code without testing in the old processor.
                         */
                        regs[95] = 0x00040040;  /* Avoid CONVERT trap + MULTIPLY trap */
                        write_word(0x800097fc, 0x70406161); /* NOP */
                        /*                        write_word(0x800097fc, 0x05005f04);  gr95 = 0x00040000 this bit avoids the trap */
                    }
                }
            }
            /* !!! Add masks */
            break;
        case 0xd7:  /* EMULATE */
            special[129] = REG_AA(_RA) * 4;
            special[130] = REG_BA(_RB) * 4;
            trap((instruction >> 16) & 0xff);
            break;
        case 0xe0:  /* MULTIPLY */
#if AM29050
            e = REG_A;
            f = REG_B;
            shift = (int64_t) e * (int64_t) f;
            REG_C = (uint32_t) shift;
            break;
#endif
        case 0xe2:  /* MULTIPLU */
#if AM29050
            c = REG_A;
            d = REG_B;
            shift = (uint64_t) c * (uint64_t) d;
            REG_C = (uint32_t) shift;
            break;
#endif
        case 0xde:  /* MULTM */
#if AM29050
            e = REG_A;
            f = REG_B;
            shift = (int64_t) e * (int64_t) f;
            REG_C = (uint32_t) (shift >> 32);
            break;
#endif
        case 0xdf:  /* MULTMU */
#if AM29050
            c = REG_A;
            d = REG_B;
            shift = (uint64_t) c * (uint64_t) d;
            REG_C = (uint32_t) (shift >> 32);
            break;
#endif
        case 0xea:  /* FEQ */
#if AM29050
            fa = reg2float(REG_A);
            fb = reg2float(REG_B);
            REG_C = (fa == fb) ? AM29K_TRUE : AM29K_FALSE;
            break;
#endif
        case 0xeb:  /* DEQ */
#if AM29050
            c = REG_AA(_RA);
            d = c + 1;
            if (d == 256)
                d = 128;
            da = reg2double(regs[c], regs[d]);
            c = REG_BA(_RB);
            d = c + 1;
            if (d == 256)
                d = 128;
            db = reg2double(regs[c], regs[d]);
            REG_C = (da == db) ? AM29K_TRUE : AM29K_FALSE;
            break;
#endif
        case 0xec:  /* FGT */
#if AM29050
            fa = reg2float(REG_A);
            fb = reg2float(REG_B);
            REG_C = (fa > fb) ? AM29K_TRUE : AM29K_FALSE;
            break;
#endif
        case 0xed:  /* DGT */
#if AM29050
            c = REG_AA(_RA);
            d = c + 1;
            if (d == 256)
                d = 128;
            da = reg2double(regs[c], regs[d]);
            c = REG_BA(_RB);
            d = c + 1;
            if (d == 256)
                d = 128;
            db = reg2double(regs[c], regs[d]);
            REG_C = (da > db) ? AM29K_TRUE : AM29K_FALSE;
            break;
#endif
        case 0xee:  /* FGE */
#if AM29050
            fa = reg2float(REG_A);
            fb = reg2float(REG_B);
            REG_C = (fa >= fb) ? AM29K_TRUE : AM29K_FALSE;
            break;
#endif
        case 0xef:  /* DGE */
#if AM29050
            c = REG_AA(_RA);
            d = c + 1;
            if (d == 256)
                d = 128;
            da = reg2double(regs[c], regs[d]);
            c = REG_BA(_RB);
            d = c + 1;
            if (d == 256)
                d = 128;
            db = reg2double(regs[c], regs[d]);
            REG_C = (da >= db) ? AM29K_TRUE : AM29K_FALSE;
            break;
#endif
        case 0xf0:  /* FADD */
#if AM29050
            fa = reg2float(REG_A);
            fb = reg2float(REG_B);
            fc = fa + fb;
            REG_C = float2reg(fc);
            break;
#endif
        case 0xf2:  /* FSUB */
#if AM29050
            fa = reg2float(REG_A);
            fb = reg2float(REG_B);
            fc = fa - fb;
            REG_C = float2reg(fc);
            break;
#endif
        case 0xf4:  /* FMUL */
#if AM29050
            fa = reg2float(REG_A);
            fb = reg2float(REG_B);
            fc = fa * fb;
            REG_C = float2reg(fc);
            break;
#endif
        case 0xf6:  /* FDIV */
#if AM29050
            fa = reg2float(REG_A);
            fb = reg2float(REG_B);
            fc = fa / fb;
            REG_C = float2reg(fc);
            break;
#endif
        case 0xf1:  /* DADD */
#if AM29050
            c = REG_AA(_RA);
            d = c + 1;
            if (d == 256)
                d = 128;
            da = reg2double(regs[c], regs[d]);
            c = REG_BA(_RB);
            d = c + 1;
            if (d == 256)
                d = 128;
            db = reg2double(regs[c], regs[d]);
            dc = da + db;
            c = REG_CA(_RC);
            d = c + 1;
            if (d == 256)
                d = 128;
            double2reg(dc, &regs[c], &regs[d]);
            break;
#endif
        case 0xf3:  /* DSUB */
#if AM29050
            c = REG_AA(_RA);
            d = c + 1;
            if (d == 256)
                d = 128;
            da = reg2double(regs[c], regs[d]);
            c = REG_BA(_RB);
            d = c + 1;
            if (d == 256)
                d = 128;
            db = reg2double(regs[c], regs[d]);
            dc = da - db;
            c = REG_CA(_RC);
            d = c + 1;
            if (d == 256)
                d = 128;
            double2reg(dc, &regs[c], &regs[d]);
            break;
#endif
        case 0xf5:  /* DMUL */
#if AM29050
            c = REG_AA(_RA);
            d = c + 1;
            if (d == 256)
                d = 128;
            da = reg2double(regs[c], regs[d]);
            c = REG_BA(_RB);
            d = c + 1;
            if (d == 256)
                d = 128;
            db = reg2double(regs[c], regs[d]);
            dc = da * db;
            c = REG_CA(_RC);
            d = c + 1;
            if (d == 256)
                d = 128;
            double2reg(dc, &regs[c], &regs[d]);
            break;
#endif
        case 0xf7:  /* DDIV */
#if AM29050
            c = REG_AA(_RA);
            d = c + 1;
            if (d == 256)
                d = 128;
            da = reg2double(regs[c], regs[d]);
            c = REG_BA(_RB);
            d = c + 1;
            if (d == 256)
                d = 128;
            db = reg2double(regs[c], regs[d]);
            dc = da / db;
            c = REG_CA(_RC);
            d = c + 1;
            if (d == 256)
                d = 128;
            double2reg(dc, &regs[c], &regs[d]);
            break;
#endif
        case 0xe4:  /* CONVERT */
#if AM29050
            if ((instruction & 0x03) == 0) {    /* Integer... */
                if ((instruction & 0x0c) == 4) { /* To float */
                    if ((instruction & 0x80) == 0) {    /* Signed */
                        e = REG_A;
                        fc = e;
                    } else {
                        c = REG_A;
                        fc = c;
                    }
                    REG_C = float2reg(fc);
                } else if ((instruction & 0x0c) == 8) { /* To double */
                    if ((instruction & 0x80) == 0) {    /* Signed */
                        e = REG_A;
                        dc = e;
                    } else {
                        c = REG_A;
                        dc = c;
                    }
                    c = REG_CA(_RC);
                    d = c + 1;
                    if (d == 256)
                        d = 128;
                    double2reg(dc, &regs[c], &regs[d]);
                }
            } else if ((instruction & 0x03) == 1) { /* Float... */
                if ((instruction & 0x0c) == 0) {    /* To integer */
                    fa = reg2float(REG_A);
                    /* !!! Round mode */
                    if ((instruction & 0x80) == 0) {  /* Signed */
                        e = fa;
                        REG_C = e;
                    } else {
                        c = fa;
                        REG_C = c;
                    }
                } else if ((instruction & 0x0c) == 8) { /* To double */
                    dc = reg2float(REG_A);
                    c = REG_CA(_RC);
                    d = c + 1;
                    if (d == 256)
                        d = 128;
                    double2reg(dc, &regs[c], &regs[d]);
                }
            } else if ((instruction & 0x03) == 2) { /* Double... */
                if ((instruction & 0x0c) == 0) {    /* To integer */
                    c = REG_AA(_RA);
                    d = c + 1;
                    if (d == 256)
                        d = 128;
                    dc = reg2double(regs[c], regs[d]);
                    /* !!! Round mode */
                    if ((instruction & 0x80) == 0) {  /* Signed */
                        e = dc;
                        REG_C = e;
                    } else {
                        c = dc;
                        REG_C = c;
                    }
                } else if ((instruction & 0x0c) == 4) { /* To float */
                    c = REG_AA(_RA);
                    d = c + 1;
                    if (d == 256)
                        d = 128;
                    fc = reg2double(regs[c], regs[d]);
                    REG_C = float2reg(fc);
                }
            }
            break;
#endif
        case 0xe5:  /* SQRT */
#if AM29050
            if ((instruction & 0x03) == 1) {
                fa = reg2float(REG_A);
                fc = sqrt(fa);
                REG_C = float2reg(fc);
            } else if ((instruction & 0x03) == 2) {
                c = REG_AA(_RA);
                d = c + 1;
                if (d == 256)
                    d = 128;
                da = reg2double(regs[c], regs[d]);
                dc = sqrt(da);
                c = REG_CA(_RC);
                d = c + 1;
                if (d == 256)
                    d = 128;
                double2reg(dc, &regs[c], &regs[d]);
            }
            break;
#endif
        case 0xe1:  /* DIVIDE */
        case 0xe3:  /* DIVIDU */
        case 0xe6:  /* CLASS */
        case 0xe8:  /* MTACC */
        case 0xe9:  /* MFACC */
        case 0xf9:  /* FDMUL */
            special[128] = REG_CA(_RC) * 4;
            special[129] = REG_AA(_RA) * 4;
            special[130] = REG_BA(_RB) * 4;
#if AM29050
            special[164] = (instruction >> 24) & 0xff;  /* EXOP */
#endif
            trap((instruction >> 24) & 0x3f);
            break;
        case 0xfc:  /* Services */
            switch ((instruction >> 8) & 0xff) {
                case 0x01:
                    pc0 = REG_B;
                    /* Read disk, gr111=track and head, gr113=target address, gr77=bytes */
                    if (debug) {
                        fprintf(stderr, "Reading track %d to 0x%08x\n", regs[111], regs[113]);
                    }
                    d = regs[113];
                    f = (regs[77] > 9216) ? 9216 : regs[77];
                    fseek(floppy, regs[111] * 9216, SEEK_SET);
                    for (e = 0; e < f; e += 4) {
                        fread(buffer, 1, 4, floppy);
                        instruction = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
                        write_word(d, instruction);
                        d += 4;
                    }
                    break;
                case 0x02:
                    pc0 = REG_B;
                    floppy_scsi(regs[REG_AA(130)], regs[REG_AA(131)], regs[REG_AA(132)], regs[REG_AA(133)], regs[REG_AA(134)], regs[REG_AA(135)]);
                    break;
#if 0   /* Assembler debugging */
                case 0xff:
                    c = regs[REG_AA(0x86)];
                    if (c) {
                        char buffer[256];
                        char *ap;
                        
                        d = c + 12;
                        ap = buffer;
                        while (*ap++ = read_byte(d))
                            d++;
                        fprintf(stderr, "Tipo 0x%08x, pos 0x%08x '%s'\n", read_word(c + 4), read_word(c + 8), buffer);
                    }
                    d = 0x00;
                    regs[98] = (c != d) ? AM29K_TRUE : AM29K_FALSE;
                    break;
#endif
                    /* Fénix definitions
                     ** #define EHOSTDOWN         (-34)
                     ** #define EINTR             (-34)
                     ** #define EWOULDBLOCK       (-33)
                     ** #define ECONNABORTED      (-32)
                     ** #define ETIMEDOUT         (-31)
                     */
#ifdef _WIN32
                case 0x15:  /* resolver (solve DNS name) */
                    pc0 = REG_B;
                    c = regs[REG_AA(0x82)]; /* Get name */
                {
                    struct addrinfo hints, *result, *rp;
                    int s;
                    char hostname[256];
                    char *ap;
                    
                    ap = hostname;
                    while (ap < hostname + 255) {
                        *ap++ = read_byte(c);
                        c++;
                    }
                    *ap = '\0';
                    
                    c = dns_read_cache(hostname);
                    if (c != 0) {
                        regs[96] = c;
                    } else {
                        /* Returns -1 for non-existent */
                        /* Returns host order domain number */
                        
                        memset(&hints, 0, sizeof(hints));
                        hints.ai_family = AF_INET;  /* ipv4 */
                        hints.ai_socktype = SOCK_STREAM;
                        
                        s = getaddrinfo(hostname, NULL, &hints, &result);
                        if (s != 0) {
                            regs[96] = -34;
                        } else {
                            struct sockaddr_in *ipv4;
                            
                            rp = result;
                            ipv4 = (struct sockaddr_in *) rp->ai_addr;
                            regs[96] = ipv4->sin_addr.s_addr;
                            dns_write_cache(hostname, regs[96]);
                        }
                    }
                    fprintf(stderr, "Solving %s to 0x%08x, returning to 0x%08x\n", hostname, regs[96], regs[REG_AA(0x80)]);
                }
                    break;
                case 0x1b:  /* tcp_abrir */
                    pc0 = REG_B;
                    c = regs[REG_AA(0x82)]; /* Source port !!! */
                    d = regs[REG_AA(0x83)]; /* IP address */
                    e = regs[REG_AA(0x84)]; /* Target port */
                {
                    SOCKET s;
                    struct sockaddr_in sserver;
                    
                    s = socket(AF_INET, SOCK_STREAM, 0);
                    if (s == INVALID_SOCKET) {
                        regs[96] = -1;  /* !!! */
                    } else {
						static DWORD ok = 1;
						int result;

                        sserver.sin_family = AF_INET;
                        sserver.sin_addr.s_addr = d;
                        sserver.sin_port = htons(e);
                        if (connect(s, (struct sockaddr *) &sserver, sizeof(sserver)) != 0) {
                            f = WSAGetLastError();
                            if (f == WSAEWOULDBLOCK || f == WSAEINTR || f == WSAEINPROGRESS || f == WSAEALREADY)
                                f = -33;    /* My OS value for EWOULDBLOCK */
                            else if (f == WSAENETDOWN || f == WSAENETUNREACH || f == WSAEHOSTUNREACH)
                                f = -34;
                            else if (f == WSAECONNREFUSED)
                                f = -32;
                            else /*if (f == WSAETIMEDOUT)*/
                                f = -31;
                            closesocket(s);
                            regs[96] = f;
                        } else {
                            regs[96] = (uint32_t) s;
                            f = 1;  /* Non-blocking mode enabled */
                            result = ioctlsocket(s, FIONBIO, &f);
                            /* Assume it worked */
                        }
                    }
                    fprintf(stderr, "tcp_abrir(0x%08x, 0x%08x, 0x%08x), returning 0x%08x\n", c, d, e, regs[96]);
                }
                    break;
                case 0x1d:  /* tcp_leer */
                    pc0 = REG_B;
                    c = regs[REG_AA(0x82)]; /* Socket */
                    d = regs[REG_AA(0x83)]; /* Address */
                    e = regs[REG_AA(0x84)]; /* Bytes */
                {
                    SOCKET s;
                    unsigned char *buffer;
                    
                    buffer = malloc(e + 1);
                    s = (SOCKET) c;
                    f = recv(s, buffer, e, 0);
                    if (f < 0) {
                        f = WSAGetLastError();
                        if (f == WSAEWOULDBLOCK || f == WSAEINTR || f == WSAEINPROGRESS || f == WSAEALREADY)
                            f = -33;    /* My OS value for EWOULDBLOCK */
                        else if (f == WSAENETDOWN || f == WSAENETUNREACH || f == WSAEHOSTUNREACH)
                            f = -34;
                        else if (f == WSAECONNREFUSED)
                            f = -32;
                        else /*if (f == WSAETIMEDOUT)*/
                            f = -31;
                    } else {
                        for (e = 0; e < f; e++) {
                            write_byte(d, buffer[e]);
                            d++;
                        }
                    }
                    regs[96] = f;
                    fprintf(stderr, "tcp_leer(0x%08x, 0x%08x, 0x%08x), returning 0x%08x\n", c, d, e, regs[96]);
                    free(buffer);
                }
                    break;
                case 0x1e:  /* tcp_mandar */
                    pc0 = REG_B;
                    c = regs[REG_AA(0x82)]; /* Socket */
                    d = regs[REG_AA(0x83)]; /* Address */
                    e = regs[REG_AA(0x84)]; /* Bytes */
                {
                    SOCKET s;
                    unsigned char *buffer;
                    
                    buffer = malloc(e + 1);
                    s = (SOCKET) c;
                    f = e;
                    for (e = 0; e < f; e++) {
                        buffer[e] = read_byte(d);
                        d++;
                    }
                    buffer[e] = '\0';
                    f = send(s, buffer, e, 0);
                    if (f == SOCKET_ERROR) {
                        f = WSAGetLastError();
                        if (f == WSAEWOULDBLOCK || f == WSAEINTR || f == WSAEINPROGRESS || f == WSAEALREADY)
                            f = -33;    /* My OS value for EWOULDBLOCK */
                        else if (f == WSAENETDOWN || f == WSAENETUNREACH || f == WSAEHOSTUNREACH)
                            f = -34;
                        else if (f == WSAECONNREFUSED)
                            f = -32;
                        else /*if (f == WSAETIMEDOUT)*/
                            f = -31;
                    }
                    regs[96] = f;
                    fprintf(stderr, "tcp_mandar(0x%08x, 0x%08x, 0x%08x) '%s', returning 0x%08x\n", c, d, e, buffer, regs[96]);
                    free(buffer);
                }
                    break;
                case 0x1f:  /* tcp_vaciar */
                    pc0 = REG_B;
                    c = regs[REG_AA(0x82)]; /* Socket */
                {
                    SOCKET s;
                    
                    s = (SOCKET) c;
                }
                    break;
                case 0x20:  /* tcp_cerrar */
                    pc0 = REG_B;
                    c = regs[REG_AA(0x82)]; /* Socket */
                {
                    SOCKET s;
                    
                    s = (SOCKET) c;
                    closesocket(s);
                    fprintf(stderr, "tcp_cerrar(0x%08x)\n", c);
                }
                    break;
                case 0x21:  /* tcp_aborta */
                    pc0 = REG_B;
                    c = regs[REG_AA(0x82)]; /* Socket */
                {
                    SOCKET s;
                        
                    s = (SOCKET) c;
                    closesocket(s);
                    fprintf(stderr, "tcp_aborta(0x%08x)\n", c);
                }
                    break;
#endif
#ifdef __APPLE__
                case 0x15:  /* resolver (solve DNS name) */
                    pc0 = REG_B;
                    c = regs[REG_AA(0x82)]; /* Get name */
                {
                    struct addrinfo hints, *result, *rp;
                    int s;
                    char hostname[256];
                    char *ap;
                    
                    ap = hostname;
                    while (ap < hostname + 255) {
                        *ap++ = read_byte(c);
                        c++;
                    }
                    *ap = '\0';
                    c = dns_read_cache(hostname);
                    if (c != 0) {
                        regs[96] = c;
                    } else {
                        /* Returns -1 for non-existent */
                        /* Returns host order domain number */
                        
                        memset(&hints, 0, sizeof(hints));
                        hints.ai_family = AF_INET;  /* ipv4 */
                        hints.ai_socktype = SOCK_STREAM;
                        
                        s = getaddrinfo(hostname, NULL, &hints, &result);
                        if (s != 0) {
                            regs[96] = -1;
                        } else {
                            struct sockaddr_in *ipv4;
                            
                            rp = result;
                            ipv4 = (struct sockaddr_in *) rp->ai_addr;
                            regs[96] = ipv4->sin_addr.s_addr;
                            dns_write_cache(hostname, regs[96]);
                        }
                    }
                    fprintf(stderr, "Solving %s to 0x%08x, returning to 0x%08x\n", hostname, regs[96], regs[REG_AA(0x80)]);
                }
                    break;
                case 0x1b:  /* tcp_abrir */
                    pc0 = REG_B;
                    c = regs[REG_AA(0x82)]; /* Source port !!! */
                    d = regs[REG_AA(0x83)]; /* IP address */
                    e = regs[REG_AA(0x84)]; /* Target port */
                {
                    int s;
                    struct sockaddr_in sserver;
                    
                    s = socket(AF_INET, SOCK_STREAM, 0);
                    if (s < 0) {
                        regs[96] = -1;  /* !!! */
                    } else {
                        sserver.sin_family = AF_INET;
                        sserver.sin_addr.s_addr = d;
                        sserver.sin_port = htons(e);
                        if (connect(s, (struct sockaddr *) &sserver, sizeof(sserver)) != 0) {
                            close(s);
                            regs[96] = -1;  /* !!! */
                        } else {
                            regs[96] = s;
                        }
                    }
                    fprintf(stderr, "tcp_abrir(0x%08x, 0x%08x, 0x%08x), returning 0x%08x\n", c, d, e, regs[96]);
                }
                    break;
                case 0x1d:  /* tcp_leer */
                    pc0 = REG_B;
                    c = regs[REG_AA(0x82)]; /* Socket */
                    d = regs[REG_AA(0x83)]; /* Address */
                    e = regs[REG_AA(0x84)]; /* Bytes */
                {
                    int s;
                    unsigned char *buffer;
                    
                    buffer = malloc(e + 1);
                    s = c;
                    f = read(s, buffer, e);
                    if (f < 0) {
                        fprintf(stderr, "errno = %d\n", errno);
                        if (errno == EWOULDBLOCK || errno == EINTR)
                            f = -33;    /* My OS value for EWOULDBLOCK */
                        else
                            f = -1;
                    } else {
                        for (e = 0; e < f; e++) {
                            write_byte(d, buffer[e]);
                            d++;
                        }
                    }
                    regs[96] = f;
                    fprintf(stderr, "tcp_leer(0x%08x, 0x%08x, 0x%08x), returning 0x%08x\n", c, d, e, regs[96]);
                    free(buffer);
                }
                    break;
                case 0x1e:  /* tcp_mandar */
                    pc0 = REG_B;
                    c = regs[REG_AA(0x82)]; /* Socket */
                    d = regs[REG_AA(0x83)]; /* Address */
                    e = regs[REG_AA(0x84)]; /* Bytes */
                {
                    int s;
                    unsigned char *buffer;
                    
                    buffer = malloc(e + 1);
                    s = c;
                    f = e;
                    for (e = 0; e < f; e++) {
                        buffer[e] = read_byte(d);
                        d++;
                    }
                    buffer[e] = '\0';
                    f = write(s, buffer, e);
                    regs[96] = f;
                    fprintf(stderr, "tcp_mandar(0x%08x, 0x%08x, 0x%08x) '%s', returning 0x%08x\n", c, d, e, buffer, regs[96]);
                    free(buffer);
                }
                    break;
                case 0x1f:  /* tcp_vaciar */
                    pc0 = REG_B;
                    c = regs[REG_AA(0x82)]; /* Socket */
                {
                    int s;
                    
                    s = c;
                }
                    break;
                case 0x20:  /* tcp_cerrar */
                    pc0 = REG_B;
                    c = regs[REG_AA(0x82)]; /* Socket */
                {
                    int s;
                    
                    s = c;
                    close(s);
                    fprintf(stderr, "tcp_cerrar(0x%08x)\n", c);
                    }
                    break;
                case 0x21:  /* tcp_aborta */
                    pc0 = REG_B;
                    c = regs[REG_AA(0x82)]; /* Socket */
                    {
                        int s;
                        
                        s = c;
                        close(s);
                        fprintf(stderr, "tcp_aborta(0x%08x)\n", c);
                    }
                    break;
#endif
                default:
                    fprintf(stderr, "Unknown service requested 0x%02x (PC = 0x%08x, lr2 = 0x%08x, lr0 = 0x%08x)\n", (instruction >> 8) & 0xff, pc2, regs[REG_AA(0x82)], regs[REG_AA(0x80)]);
                    debug_info();
                    exit(1);
            }
            break;
        default:
            fprintf(stderr, "Instruction 0x%08x not implemented (PC = 0x%08x)\n", instruction, pc1);
            debug_info();
            exit(1);
    }
}
