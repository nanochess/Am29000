/*
 ** CL-GD5440 emulation
 **
 ** by Oscar Toledo G.
 **
 ** Creation date: Aug/10/2026. Based on the CL-GD5429 driver.
 ** Revision date: Aug/17/2026. Added masks for RAM size.
 */

#define CLGD5440_RAM_SIZE  2097152
#define CLGD5440_RAM_MASK  0x1fffff

struct cl_gd5440 {
    int feature;    /* Feature control */
    int mstatus;    /* Miscellaneous status */
    int cstatus;    /* CRTC status */
    int dac_read;
    int dac_mask;
    int hidden_dac;
    int sequencer_register;
    int sr[32];
    int crtc_register;
    int cr[64];
    int graphics_register;
    int gr[64];
    int attribute_switch;
    int attribute_register;
    int ar[32];
    int palette_pointer;
    int palette[768];
    int cursor_x;
    int cursor_y;
    unsigned char cursor[64 * 32];
    unsigned char ram[CLGD5440_RAM_SIZE];
};

extern struct cl_gd5440 vga_pci;

extern int clgd5440_pci_io_write_byte(int, int);
extern int clgd5440_pci_io_write_word(int, int);
extern int clgd5440_pci_io_read_byte(int);
extern int clgd5440_pci_mem_write_byte(int, int);
extern int clgd5440_pci_mem_write_word(int, int);
extern int clgd5440_pci_mem_write_dword(int, int);
extern int clgd5440_pci_mem_read_word(int);
extern int clgd5440_pci_mem_read_dword(int);
extern void clgd5440_save_cursor(void);
extern void clgd5440_restore_cursor(void);
extern void clgd5440_draw_cursor(void);
extern void clgd5440_dump_registers(void);
