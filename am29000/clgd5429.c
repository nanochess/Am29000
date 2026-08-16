/*
 ** CL-GD5429 emulation (ISA)
 **
 ** by Oscar Toledo G.
 ** https://nanochess.org/
 **
 ** Creation date: Jul/23/2026.
 ** Revision date: Jul/27/2026. Basic support for bitblt copy and XOR.
 ** Revision date: Jul/28/2026. Allows to read RAM as 16-bit words.
 **                             Added hardware cursor routines.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "clgd5429.h"

extern FILE *debug;
extern void debug_info(void);

/*
 ** This is a simple emulation of the Cirrus Logic GD5429, only the
 ** required functions for a 800x600x64k display with some
 ** acceleration functions and hardware cursor.
 **
 ** Given the slowness of the ISA bus, these features gave
 ** amazing speed for graphical operating systems.
 **
 ** Technical reference manual downloaded from:
 ** http://www.s100computers.com/My%20System%20Pages/VGA_16_Board%20(Cirrus)/GD542x%20Technical%20Reference%20Manual.pdf
 **
 ** Hardware cursor described in page 528.
 **
 ** Notice there is no support for the BIOS ROM as it isn't used.
 */

struct cl_gd5429 vga;

/*
** IO port write
*/
int clgd5429_io_write_byte(int port, int byte)
{
    uint32_t source;
    uint32_t target;
    uint32_t source_stride;
    uint32_t target_stride;
    uint32_t width;
    uint32_t height;
    
    switch (port) {
        case 0x3c0: /* Attribute registers */
            if (vga.attribute_switch == 0)
                vga.attribute_register = byte;  /* Address */
            else
                vga.ar[vga.attribute_register & 0x1f] = byte;   /* Data */
            vga.attribute_switch ^= 1;
            break;
        case 0x3c2: /* Miscellaneous output register */
            vga.mstatus = byte;
            break;
        case 0x3c4: /* Sequencer register */
            vga.sequencer_register = byte;
            break;
        case 0x3c5: /* Sequencer data */
            /*
             ** The hardware cursor coordinates use the top 3 bits of the
             ** register number as bits 2-0 of coordinate.
             */
            if ((vga.sequencer_register & 0x1f) == 0x10) {
                vga.cursor_x = (byte << 3) | (vga.sequencer_register >> 5);
            } else if ((vga.sequencer_register & 0x1f) == 0x11) {
                vga.cursor_y = (byte << 3) | (vga.sequencer_register >> 5);
            }
            vga.sr[vga.sequencer_register & 0x1f] = byte;
            break;
        case 0x3c6: /* DAC mask */
            if (vga.dac_read == 4) {
                vga.dac_read = 0;
                vga.hidden_dac = byte;
                fprintf(stderr, "Hidden DAC value set to 0x%02x\n", byte);
                if (byte == 0xe1) {
                    fprintf(stderr, "Bitmap is 5-6-5 XGA(tm)\n");
                } else {
                    fprintf(stderr, "Error: DAC value unhandled\n");
                }
                break;
            }
            vga.dac_mask = byte;
            break;
        case 0x3c8: /* DAC register */
            vga.palette_pointer = byte * 3;
            break;
        case 0x3c9: /* DAC R,G,B */
            vga.palette[vga.palette_pointer++] = byte;
            if (vga.palette_pointer == 768)
                vga.palette_pointer = 0;
            break;
        case 0x3ce: /* Graphics register */
            vga.graphics_register = byte;
            break;
        case 0x3cf: /* Graphics data */
            vga.gr[vga.graphics_register & 0x3f] = byte;
            if ((vga.graphics_register & 0x3f) == 0x31 && byte == 0x02) {   /* Start bitblt */
                /* lr8 lr2 = source address */
                /* lr9 lr3 = target address */
                /* lr10 lr4 = width */
                /* lr11 lr5 = height */
                /* lr13 lr7 = target stride */
                /* lr14 lr8 = source stride */
                /* lr15 lr9 = command (0x0d = copy, 0x59 = xor) */
                vga.gr[0x31] = 0x01;    /* Busy */
                /* vga.gr[0x30] is Mode */
                width = vga.gr[0x20] | (vga.gr[0x21] << 8);
                height = vga.gr[0x22] | (vga.gr[0x23] << 8);
                target_stride = vga.gr[0x24] | (vga.gr[0x25] << 8);
                source_stride = vga.gr[0x26] | (vga.gr[0x27] << 8);
                target = vga.gr[0x28] | (vga.gr[0x29] << 8) | (vga.gr[0x2a] << 16); /* Target address */
                if (vga.gr[0x30] & 0x40) {  /* 8x8 pattern copy */
                    int d;
                    int e;
                    int f;
                    uint32_t source2;
                    uint32_t target2;
                    
                    d = 0;
                    do {
                        source = vga.gr[0x2c] | (vga.gr[0x2d] << 8) | (vga.gr[0x2e] << 16); /* Source address */
                        source += d * 16;
                        source &= 0x0fffff;
                        target &= 0x0fffff;
                        if (source + width <= 0x100000 && target + width <= 0x100000) {
                            /* Operations with pattern in multiples of 8 */
                            if (vga.gr[0x32] == 0x0d) { /* Copy */
                                target2 = target;
                                e = width + 1;
                                do {
                                    if (e > 16)
                                        f = 16;
                                    else
                                        f = e;
                                    memcpy(&vga.ram[target2], &vga.ram[source], f);
                                    target2 += f;
                                    e -= f;
                                } while (e) ;
                            } else if (vga.gr[0x32] == 0x59) {  /* XOR */
                                target2 = target;
                                e = width + 1;
                                do {
                                    if (e > 16)
                                        f = 16;
                                    else
                                        f = e;
                                    e -= f;
                                    source2 = source;
                                    do {
                                        vga.ram[target2++] ^= vga.ram[source2++];
                                    } while (--f) ;
                                } while (e) ;
                            }
                        }
                        target += target_stride;
                        d = (d + 1) & 7;    /* Pattern row (0-7) */
                    } while (height--) ;
                } else {
                    source = vga.gr[0x2c] | (vga.gr[0x2d] << 8) | (vga.gr[0x2e] << 16); /* Source address */
                    if (vga.gr[0x30] & 1) { /* Copy in reverse direction */
                        /* Source and target addresses point to the lower-right byte */
                        do {
                            source &= 0x0fffff;
                            target &= 0x0fffff;
                            if (source - width >= 0 && target - width >= 0) {
                                unsigned char *p1 = &vga.ram[source];
                                unsigned char *p2 = &vga.ram[target];
                                uint32_t c = width + 1;

                                if (vga.gr[0x32] == 0x0d) {
                                    do {
                                        *p2-- = *p1--;
                                    } while (--c) ;
                                } else if (vga.gr[0x32] == 0x59) {
                                    do {
                                        *p2-- ^= *p1--;
                                    } while (--c) ;
                                }
                            }
                            source -= source_stride;
                            target -= target_stride;
                        } while (height--) ;
                    } else {
                        do {
                            source &= 0x0fffff;
                            target &= 0x0fffff;
                            if (source + width <= 0x100000 && target + width <= 0x100000) {
                                if (vga.gr[0x32] == 0x0d) {
                                    memcpy(&vga.ram[target], &vga.ram[source], width + 1);
                                } else if (vga.gr[0x32] == 0x59) {
                                    unsigned char *p1 = &vga.ram[source];
                                    unsigned char *p2 = &vga.ram[target];
                                    uint32_t c = width + 1;
                                    
                                    do {
                                        *p2++ ^= *p1++;
                                    } while (--c) ;
                                }
                            }
                            source += source_stride;
                            target += target_stride;
                        } while (height--) ;
                    }
                }
                vga.gr[0x31] = 0x00;    /* Not busy */
            }
            break;
        case 0x3d4: /* CRTC register */
            vga.crtc_register = byte;
            break;
        case 0x3d5: /* CRTC data */
            vga.cr[vga.crtc_register & 0x3f] = byte;
            break;
        case 0x3da: /* Feature register */
            vga.feature = byte;
            break;
        default:
            return -1;
    }
    return 0;
}

/*
 ** Write a 16-bit word to the I/O ports.
 ** It simply separates it in two bytes.
 */
int clgd5429_io_write_word(int port, int word)
{
    int c;
    
    c = clgd5429_io_write_byte(port, word & 0xff);
    if (c != -1)
        c = clgd5429_io_write_byte(port + 1, (word >> 8) & 0xff);
    return c;
}

/*
 ** Read a byte from the I/O port.
 */
int clgd5429_io_read_byte(int port)
{
    switch (port) {
        case 0x3c1: /* Attribute registers */
            vga.attribute_switch ^= 1;
            return vga.ar[vga.attribute_register & 0x1f];
        case 0x3c2: /* Miscellaneous output register */
            return vga.mstatus;
        case 0x3c4: /* Sequencer register number */
            return vga.sequencer_register;
        case 0x3c5: /* Sequencer register data */
            return vga.sr[vga.sequencer_register & 0x1f];
        case 0x3c6: /* DAC mask */
            if (vga.dac_read == 4) {
                vga.dac_read = 0;
                return vga.hidden_dac;
            }
            vga.dac_read = vga.dac_read + 1;
            return vga.dac_mask;
        case 0x3ce: /* Graphics register number */
            return vga.graphics_register;
        case 0x3cf: /* Graphics register data */
            return vga.gr[vga.graphics_register & 0x3f];
        case 0x3d4: /* CRTC register number */
            return vga.crtc_register;
        case 0x3d5: /* CRTC register data */
            return vga.cr[vga.crtc_register & 0x3f];
        case 0x3da: /* Feature register */
            return vga.cstatus;
        default:
            return -1;
    }
}

/*
 ** Write a byte into RAM.
 */
int clgd5429_mem_write_byte(int address, int byte)
{
    if (address < 0x000a0000 || address > 0x000bffff) { /* A:0000 - B:FFFF */
        return -1;
    }
    address -= 0x000a0000;
    address <<= 4;  /* Supposes vga.gr[0x0b] == 0x1c */
    if (address >= 0x100000) {    /* 1 MB of RAM */
        fprintf(stderr, "CL-GD5429: Too big address 0x%08x\n", address);
        return -1;
    }
    /* Supposes Write Mode 4. vga.gr[0x0b] & 4 opens vga.gr[0x05] & 7 */
    if ((vga.gr[0x0b] & 0x04) != 0 && (vga.gr[0x05] & 7) == 4) {    /* Write mode 4 */
        byte &= vga.sr[2];  /* Mask, and now expand bitmap to 16-bit pixels */
        if (byte & 0x80) {
            vga.ram[address + 0] = vga.gr[0x01];
            vga.ram[address + 1] = vga.gr[0x11];
        }
        if (byte & 0x40) {
            vga.ram[address + 2] = vga.gr[0x01];
            vga.ram[address + 3] = vga.gr[0x11];
        }
        if (byte & 0x20) {
            vga.ram[address + 4] = vga.gr[0x01];
            vga.ram[address + 5] = vga.gr[0x11];
        }
        if (byte & 0x10) {
            vga.ram[address + 6] = vga.gr[0x01];
            vga.ram[address + 7] = vga.gr[0x11];
        }
        if (byte & 0x08) {
            vga.ram[address + 8] = vga.gr[0x01];
            vga.ram[address + 9] = vga.gr[0x11];
        }
        if (byte & 0x04) {
            vga.ram[address + 10] = vga.gr[0x01];
            vga.ram[address + 11] = vga.gr[0x11];
        }
        if (byte & 0x02) {
            vga.ram[address + 12] = vga.gr[0x01];
            vga.ram[address + 13] = vga.gr[0x11];
        }
        if (byte & 0x01) {
            vga.ram[address + 14] = vga.gr[0x01];
            vga.ram[address + 15] = vga.gr[0x11];
        }
    } else {
        fprintf(stderr, "CL-GD5429: Unhandled write mode 0x%02x\n", vga.gr[0x0b]);
        debug_info();
        exit(1);
    }
    return 0;
}
 
/*
 ** Write a 16-bit word into RAM.
 */
int clgd5429_mem_write_word(int address, int word)
{
    if (address < 0x000a0000 || address > 0x000bffff) {
        return -1;
    }
    address -= 0x000a0000;
    if (vga.gr[0x0b] == 0x00) {
        address += (vga.gr[0x09] & 0xfc) << 12;
        if (address >= 0x100000) {    /* 1 MB of RAM */
            fprintf(stderr, "CL-GD5429: Too big address 0x%08x\n", address);
            return -1;
        }
        vga.ram[address + 0] = word;
        vga.ram[address + 1] = word >> 8;
        return 0;
    }
    fprintf(stderr, "CL-GD5429: Unhandled write mode 0x%02x\n", vga.gr[0x0b]);
    debug_info();
    exit(1);
    return 0;
}

/*
 ** Read a 16-bit word from RAM.
 */
int clgd5429_mem_read_word(int address)
{
    if (address < 0x000a0000 || address > 0x000bffff) {
        return -1;
    }
    address -= 0x000a0000;
    if (vga.gr[0x0b] == 0x00) {
        address += (vga.gr[0x09] & 0xfc) << 12;
        if (address >= 0x100000) {    /* 1 MB of RAM */
            fprintf(stderr, "CL-GD5429: Too big address 0x%08x\n", address);
            return -1;
        }
        return (vga.ram[address + 1] << 8) | vga.ram[address];
    }
    fprintf(stderr, "CL-GD5429: Unhandled read mode 0x%02x\n", vga.gr[0x0b]);
    debug_info();
    exit(1);
    return 0;
}

/*
 ** Save the background under the cursor.
 */
void clgd5429_save_cursor(void)
{
    int source;
    int y;
    
    if ((vga.sr[0x12] & 1) == 0)
        return;
    source = (vga.cursor_y * 800 + vga.cursor_x) * 2;
    for (y = 0; y < 32; y++) {
        source &= 0x0fffff;
        if (source + 64 > 0x100000)
            continue;
        memcpy(&vga.cursor[y * 64], &vga.ram[source], 64);
        source += 800 * 2;
    }
}

/*
 ** Restore the background under the cursor.
 */
void clgd5429_restore_cursor(void)
{
    int source;
    int y;

    if ((vga.sr[0x12] & 1) == 0)
        return;
    source = (vga.cursor_y * 800 + vga.cursor_x) * 2;
    for (y = 0; y < 32; y++) {
        source &= 0x0fffff;
        if (source + 64 > 0x100000)
            continue;
        memcpy(&vga.ram[source], &vga.cursor[y * 64], 64);
        source += 800 * 2;
    }
}

/*
 ** Draw the cursor
 ** Currently only 32x32 pixels supported.
 */
void clgd5429_draw_cursor(void)
{
    int source;
    int target;
    int x;
    int y;
    int c;
    int color_1;
    int color_2;
    
    if ((vga.sr[0x12] & 1) == 0)    /* Hardware cursor enabled? */
        return;
    color_1 = ((vga.palette[0] & 0x3e) << 10) | ((vga.palette[1] & 0x3f) << 5) | ((vga.palette[2] & 0x3e) >> 1);
    color_2 = ((vga.palette[765] & 0x3e) << 10) | ((vga.palette[766] & 0x3f) << 5) | ((vga.palette[767] & 0x3e) >> 1);
    for (y = 0; y < 32; y++) {
        source = 0xfc000 + (vga.sr[0x13] * 256) + y * 4;
        target = ((vga.cursor_y + y) * 800 + vga.cursor_x) * 2;
        if (target + 64 > 0x100000)
            continue;
        for (x = 0; x < 4; x++) {
            for (c = 0x80; c; c >>= 1) {
                if ((vga.ram[source + 128] & c) == 0) {
                    if ((vga.ram[source] & c) != 0) {   /* XOR pixel */
                        vga.ram[target + 0] = ~vga.ram[target + 0];
                        vga.ram[target + 1] = ~vga.ram[target + 1];
                    }
                } else {
                    if ((vga.ram[source] & c) == 0) {   /* Background color */
                        vga.ram[target + 0] = color_1;
                        vga.ram[target + 1] = color_1 >> 8;
                    } else {
                        vga.ram[target + 0] = color_2;  /* Foreground color */
                        vga.ram[target + 1] = color_2 >> 8;
                    }
                }
                target += 2;
            }
            source++;
        }
    }
}

/*
 ** Dump Cirrus Logic GD5429 registers
 ** (used before quitting in case of errors)
 */
void clgd5429_dump_registers(void)
{
    int c;
    
    fprintf(debug, "0x03c2= 0x%02x\n", vga.mstatus);
    for (c = 0; c < 32; c++) {
        fprintf(debug, "SR%X= 0x%02x%c", c, vga.sr[c], ((c & 3) == 3 ? '\n' : ' '));
    }
    for (c = 0; c < 32; c++) {
        fprintf(debug, "GR%X= 0x%02x%c", c, vga.gr[c], ((c & 3) == 3 ? '\n' : ' '));
    }
    for (c = 0; c < 64; c++) {
        fprintf(debug, "CR%X= 0x%02x%c", c, vga.cr[c], ((c & 3) == 3 ? '\n' : ' '));
    }
    for (c = 0; c < 32; c++) {
        fprintf(debug, "AR%X= 0x%02x%c", c, vga.ar[c], ((c & 3) == 3 ? '\n' : ' '));
    }
}
