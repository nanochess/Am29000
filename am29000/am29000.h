/*
** Am29000 processor emulation definitions
**
** by Oscar Toledo G.
** https://nanochess.org/
**
** Creation date: Jul/30/2026.
*/

#define read_word(addr)  ((addr) < 0x80000000 ? rom[((addr) >> 2) & 0x7fff] : memory[((addr) >> 2) & 0x1ffff])
#define write_word(addr, data)  ((addr) < 0x80000000 ? (rom[((addr) >> 2) & 0x7fff] = (data)) : (memory[((addr) >> 2) & 0x1ffff] = (data)))

#define AM29K_TRUE  0x80000000
#define AM29K_FALSE 0x00000000

/*
** There are global registers gr0, gr1, and gr64-gr127
** Undefined behavior if gr2-gr63 are used.
*/
#define _RC    ((instruction >> 16) & 0xff)
#define _RA    ((instruction >> 8) & 0xff)
#define _RB    (instruction & 0xff)

#define IMM    (instruction & 0xff)
#define IMM16 ((instruction & 0xff) | ((instruction >> 8) & 0xff00))
#define IMM16S ((instruction & 0x800000) ? IMM16 - 0x10000 : IMM16)

#define REG_CA(R)   (R == 0 ? (special[128] / 4) & 0xff : ((R < 128) ? R : ((regs[1] / 4 + R) & 0x7f) | 0x80))
#define REG_AA(R)   (R == 0 ? (special[129] / 4) & 0xff : ((R < 128) ? R : ((regs[1] / 4 + R) & 0x7f) | 0x80))
#define REG_BA(R)   (R == 0 ? (special[130] / 4) & 0xff : ((R < 128) ? R : ((regs[1] / 4 + R) & 0x7f) | 0x80))

#define REG_C    regs[REG_CA(_RC)]
#define REG_A    regs[REG_AA(_RA)]
#define REG_B    regs[REG_BA(_RB)]

/*
 ** The BP and FC fields from the ALU register are replicated
 ** in the BP and FC registers.
 */
#define READ_BP ((special[132] >> 5) & 3)
#define WRITE_BP(v) special[132] = (special[132] & ~0x60) | (((v) & 3) << 5)

#define READ_FC (special[132] & 0x1f)
#define WRITE_FC(v) special[132] = (special[132] & ~0x1f) | ((v) & 0x1f)

#define ALU_CARRY ((special[132] >> 7) & 1)

#define ALU(v1, v2, c) \
  if ((special[2] & 0x0400) == 0) { \
    uint64_t tmp = v1 + v2 + c; \
    if (tmp >> 32) \
        special[132] = special[132] | 0x80; \
    else \
        special[132] = special[132] & ~0x80; \
    if (tmp & 0x80000000u) \
        special[132] = special[132] | 0x0200; \
    else \
        special[132] = special[132] & ~0x0200; \
    if ((tmp & 0xffffffffu) == 0) \
        special[132] = special[132] | 0x0100; \
    else \
        special[132] = special[132] & ~0x0100; \
    special[132] = special[132] & ~0x0400; /* No overflow */ \
  }
    
#define ALU_SIMPLE(v) \
  if ((special[2] & 0x0400) == 0) { \
    if (v & 0x80000000) \
        special[132] = special[132] | 0x0200; \
    else \
        special[132] = special[132] & ~0x0200; \
    if (v == 0) \
        special[132] = special[132] | 0x0100; \
    else \
        special[132] = special[132] & ~0x0100; \
  }

extern uint32_t rom[32768];
extern uint32_t memory[524288 / 4];
extern uint32_t regs[256];
extern uint32_t special[256];

extern uint32_t pc0;
extern uint32_t pc1;
extern uint32_t pc2;

extern uint32_t count;

extern int read_byte(uint32_t);
extern void write_byte(uint32_t, uint32_t);
void disassemble(uint32_t, uint32_t, char *);
extern void am29000_emulate(void);

/*
 ** Required from other modules.
 */
extern FILE *floppy;
extern FILE *debug; /* emulator.c */
uint32_t read_isa(uint32_t);
void write_isaw(uint32_t, uint32_t);
void write_isa(uint32_t, uint32_t);
void floppy_scsi(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
extern void debug_info(void);

