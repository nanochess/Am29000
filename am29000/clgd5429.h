/*
 ** CL-GD5429 emulation
 **
 ** by Oscar Toledo G.
 **
 ** Creation date: Jul/23/2026.
 */

struct cl_gd5429 {
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
    unsigned char ram[1048576];
};

extern struct cl_gd5429 vga;

extern int clgd5429_io_write_byte(int, int);
extern int clgd5429_io_write_word(int, int);
extern int clgd5429_io_read_byte(int);
extern int clgd5429_mem_write_byte(int, int);
extern int clgd5429_mem_write_word(int, int);
extern int clgd5429_mem_read_word(int);
extern void clgd5429_save_cursor(void);
extern void clgd5429_restore_cursor(void);
extern void clgd5429_draw_cursor(void);
extern void clgd5429_dump_registers(void);
