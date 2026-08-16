/*
 ** CL-GD5440 emulation (PCI)
 **
 ** by Oscar Toledo G.
 ** https://nanochess.org/
 **
 ** Creation date: Aug/10/2026. Based on the CL-GD5429 driver.
 ** Revision date: Aug/11/2026. Implemented bitmap expansion.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "clgd5440.h"

extern FILE *debug;
extern void debug_info(void);

/*
 ** This is a simple emulation of the Cirrus Logic GD5440, only the
 ** required functions for a 800x600x64k display with some
 ** acceleration functions and hardware cursor.
 **
 ** Technical reference manual downloaded from:
 ** https://www.vgamuseum.info/index.php/cpu/item/143-cirrus-logic-cl-gd5440
 **
 ** MMIO (Memory Mapped I/O) described in page 566.
 **
 ** Notice there is no support for the BIOS ROM as it isn't used.
 */

struct cl_gd5440 vga_pci;

static uint32_t source;
static uint32_t target;
static uint32_t source_stride;
static uint32_t target_stride;
static uint32_t width;
static uint32_t height;
static uint32_t current_target;
static uint32_t current_width;

static void clgd5440_bitmap_expansion(uint32_t word)
{
    uint32_t address;
    int byte;
    int available_bytes;
    int mask;
    int c;
    
    address = current_target;
    available_bytes = 4;
    while (available_bytes--) {
        byte = word & 0xff;
        word >>= 8;
        mask = 0x80;
        c = current_width > 16 ? 16 : current_width;
        current_width -= c;
        while (c > 0) {
            if (byte & mask) {
                vga_pci.ram[(current_target + 0) & 0x0fffff] = vga_pci.gr[0x01];
                vga_pci.ram[(current_target + 1) & 0x0fffff] = vga_pci.gr[0x11];
            }
            mask >>= 1;
            current_target += 2;
            c -= 2;
        }
        if (current_width == 0) {
            target += target_stride;
            current_target = target;
            current_width = width + 1;
            if (height == 0) {
                vga_pci.gr[0x31] = 0x00;    /* Not busy */
            } else {
                height--;
            }
        }
    }
}

static void clgd5440_start_bitblt(void)
{
    /* lr8 lr2 = source address */
    /* lr9 lr3 = target address */
    /* lr10 lr4 = width */
    /* lr11 lr5 = height */
    /* lr13 lr7 = target stride */
    /* lr14 lr8 = source stride */
    /* lr15 lr9 = command (0x0d = copy, 0x59 = xor) */
    vga_pci.gr[0x31] = 0x01;    /* Busy */
    /* vga_pci.gr[0x30] is Mode */
    width = vga_pci.gr[0x20] | (vga_pci.gr[0x21] << 8);
    height = vga_pci.gr[0x22] | (vga_pci.gr[0x23] << 8);
    target_stride = vga_pci.gr[0x24] | (vga_pci.gr[0x25] << 8);
    source_stride = vga_pci.gr[0x26] | (vga_pci.gr[0x27] << 8);
    target = vga_pci.gr[0x28] | (vga_pci.gr[0x29] << 8) | (vga_pci.gr[0x2a] << 16); /* Target address */
    if (vga_pci.gr[0x30] == 0x9c) { /* Bitmap expansion */
        current_target = target;
        current_width = width + 1;
        return;
    }
    if (vga_pci.gr[0x30] & 0x40) {  /* 8x8 pattern copy */
        int d;
        int e;
        int f;
        uint32_t source2;
        uint32_t target2;
        
        d = 0;
        do {
            source = vga_pci.gr[0x2c] | (vga_pci.gr[0x2d] << 8) | (vga_pci.gr[0x2e] << 16); /* Source address */
            source += d * 16;
            source &= 0x0fffff;
            target &= 0x0fffff;
            if (source + width <= 0x100000 && target + width <= 0x100000) {
                /* Operations with pattern in multiples of 8 */
                if (vga_pci.gr[0x32] == 0x0d) { /* Copy */
                    target2 = target;
                    e = width + 1;
                    do {
                        if (e > 16)
                            f = 16;
                        else
                            f = e;
                        memcpy(&vga_pci.ram[target2], &vga_pci.ram[source], f);
                        target2 += f;
                        e -= f;
                    } while (e) ;
                } else if (vga_pci.gr[0x32] == 0x59) {  /* XOR */
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
                            vga_pci.ram[target2++] ^= vga_pci.ram[source2++];
                        } while (--f) ;
                    } while (e) ;
                }
            }
            target += target_stride;
            d = (d + 1) & 7;    /* Pattern row (0-7) */
        } while (height--) ;
    } else {
        source = vga_pci.gr[0x2c] | (vga_pci.gr[0x2d] << 8) | (vga_pci.gr[0x2e] << 16); /* Source address */
        if (vga_pci.gr[0x30] & 1) { /* Copy in reverse direction */
            /* Source and target addresses point to the lower-right byte */
            do {
                source &= 0x0fffff;
                target &= 0x0fffff;
                if (source - width >= 0 && target - width >= 0) {
                    unsigned char *p1 = &vga_pci.ram[source];
                    unsigned char *p2 = &vga_pci.ram[target];
                    uint32_t c = width + 1;

                    if (vga_pci.gr[0x32] == 0x0d) {
                        do {
                            *p2-- = *p1--;
                        } while (--c) ;
                    } else if (vga_pci.gr[0x32] == 0x59) {
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
                    if (vga_pci.gr[0x32] == 0x0d) {
                        memcpy(&vga_pci.ram[target], &vga_pci.ram[source], width + 1);
                    } else if (vga_pci.gr[0x32] == 0x59) {
                        unsigned char *p1 = &vga_pci.ram[source];
                        unsigned char *p2 = &vga_pci.ram[target];
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
    vga_pci.gr[0x31] = 0x00;    /* Not busy */
}

/*
** IO port write
*/
int clgd5440_pci_io_write_byte(int port, int byte)
{
    byte >>= 8 * (port & 3);
    byte &= 0xff;
    switch (port) {
        case 0x3c0: /* Attribute registers */
            if (vga_pci.attribute_switch == 0)
                vga_pci.attribute_register = byte;  /* Address */
            else
                vga_pci.ar[vga_pci.attribute_register & 0x1f] = byte;   /* Data */
            vga_pci.attribute_switch ^= 1;
            break;
        case 0x3c2: /* Miscellaneous output register */
            vga_pci.mstatus = byte;
            break;
        case 0x3c4: /* Sequencer register */
            vga_pci.sequencer_register = byte;
            break;
        case 0x3c5: /* Sequencer data */
            /*
             ** The hardware cursor coordinates use the top 3 bits of the
             ** register number as bits 2-0 of coordinate.
             */
            if ((vga_pci.sequencer_register & 0x1f) == 0x10) {
                vga_pci.cursor_x = (byte << 3) | (vga_pci.sequencer_register >> 5);
            } else if ((vga_pci.sequencer_register & 0x1f) == 0x11) {
                vga_pci.cursor_y = (byte << 3) | (vga_pci.sequencer_register >> 5);
            }
            vga_pci.sr[vga_pci.sequencer_register & 0x1f] = byte;
            break;
        case 0x3c6: /* DAC mask */
            if (vga_pci.dac_read == 4) {
                vga_pci.dac_read = 0;
                vga_pci.hidden_dac = byte;
                fprintf(stderr, "Hidden DAC value set to 0x%02x\n", byte);
                if (byte == 0xe1) {
                    fprintf(stderr, "Bitmap is 5-6-5 XGA(tm)\n");
                } else {
                    fprintf(stderr, "Error: DAC value unhandled\n");
                }
                break;
            }
            vga_pci.dac_mask = byte;
            break;
        case 0x3c8: /* DAC register */
            vga_pci.palette_pointer = byte * 3;
            break;
        case 0x3c9: /* DAC R,G,B */
            vga_pci.palette[vga_pci.palette_pointer++] = byte;
            if (vga_pci.palette_pointer == 768)
                vga_pci.palette_pointer = 0;
            break;
        case 0x3ce: /* Graphics register */
            vga_pci.graphics_register = byte;
            break;
        case 0x3cf: /* Graphics data */
            vga_pci.gr[vga_pci.graphics_register & 0x3f] = byte;
            if ((vga_pci.graphics_register & 0x3f) == 0x31 && byte == 0x02) {   /* Start bitblt */
                clgd5440_start_bitblt();
            }
            break;
        case 0x3d4: /* CRTC register */
            vga_pci.crtc_register = byte;
            break;
        case 0x3d5: /* CRTC data */
            vga_pci.cr[vga_pci.crtc_register & 0x3f] = byte;
            break;
        case 0x3da: /* Feature register */
            vga_pci.feature = byte;
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
int clgd5440_pci_io_write_word(int port, int word)
{
    int c;
    
    c = clgd5440_pci_io_write_byte(port, word);
    if (c != -1)
        c = clgd5440_pci_io_write_byte(port + 1, word);
    return c;
}

/*
 ** Read a byte from the I/O port.
 **
 ** The bus positioning is done by the caller.
 */
int clgd5440_pci_io_read_byte(int port)
{
    switch (port) {
        case 0x3c1: /* Attribute registers */
            vga_pci.attribute_switch ^= 1;
            return vga_pci.ar[vga_pci.attribute_register & 0x1f];
        case 0x3c2: /* Miscellaneous output register */
            return vga_pci.mstatus;
        case 0x3c4: /* Sequencer register number */
            return vga_pci.sequencer_register;
        case 0x3c5: /* Sequencer register data */
            return vga_pci.sr[vga_pci.sequencer_register & 0x1f];
        case 0x3c6: /* DAC mask */
            if (vga_pci.dac_read == 4) {
                vga_pci.dac_read = 0;
                return vga_pci.hidden_dac;
            }
            vga_pci.dac_read = vga_pci.dac_read + 1;
            return vga_pci.dac_mask;
        case 0x3ce: /* Graphics register number */
            return vga_pci.graphics_register;
        case 0x3cf: /* Graphics register data */
            return vga_pci.gr[vga_pci.graphics_register & 0x3f];
        case 0x3d4: /* CRTC register number */
            return vga_pci.crtc_register;
        case 0x3d5: /* CRTC register data */
            return vga_pci.cr[vga_pci.crtc_register & 0x3f];
        case 0x3da: /* Feature register */
            return vga_pci.cstatus;
        default:
            return -1;
    }
}

/*
 ** Write a byte into RAM.
 */
int clgd5440_pci_mem_write_byte(int address, int word)
{
    if ((address & 0xffff8000) == 0x800b8000) { /* MMIO registers */
        address &= 0xfc;
        switch (address) {
            default:
                fprintf(stderr, "CL-GD5440: Unhandled byte write address 0x%08x\n", address);
                debug_info();
                exit(1);
                break;
            case 0x40:
                vga_pci.gr[0x31] = word & 0xff;
                if ((word & 0xff) == 0x02) {
                    clgd5440_start_bitblt();
                }
                break;
        }
        return 0;
    }
    fprintf(stderr, "CL-GD5440: Unhandled byte write address 0x%08x\n", address);
    debug_info();
    exit(1);
    return 0;
}
 
/*
 ** Write a 16-bit word into RAM.
 */
int clgd5440_pci_mem_write_word(int address, int word)
{
    if ((address & 0xff000000) == 0x81000000) { /* 16 mb. aperture */
        if ((address & 2) == 0) {
            address &= 0x000ffffc;
            vga_pci.ram[address] = word;
            vga_pci.ram[address + 1] = word >> 8;
        } else {
            address &= 0x000ffffc;
            vga_pci.ram[address + 2] = word >> 16;
            vga_pci.ram[address + 3] = word >> 24;
        }
        return 0;
    }
    fprintf(stderr, "CL-GD5440: Unhandled 16-bit write to 0x%08x\n", address);
    debug_info();
    exit(1);
    return 0;
}

/*
 ** Write a 32-bit word
 */
int clgd5440_pci_mem_write_dword(int address, int word)
{
    if ((address & 0xc0000000) == 0xc0000000) {
        if ((address & 0x08000000) == 0x08000000) {
            /* !!! Ignore PCI header writes that enable the video card */
            return 0;
        }
        return 0;
    }
    if ((address & 0xff000000) == 0x81000000) { /* 16 mb. aperture */
        if (vga_pci.gr[0x31] == 0x01) { /* In this emulation subset, it must be bitmap expansion */
            clgd5440_bitmap_expansion(word);
            return 0;
        }
        address &= 0x000ffffc;
        vga_pci.ram[address] = word;
        vga_pci.ram[address + 1] = word >> 8;
        vga_pci.ram[address + 2] = word >> 16;
        vga_pci.ram[address + 3] = word >> 24;
        return 0;
    }
    if ((address & 0xffff8000) == 0x800b8000) { /* MMIO registers */
        address &= 0xfc;
        switch (address) {
            case 0x00:
                vga_pci.gr[0x00] = word & 0xff;
                vga_pci.gr[0x10] = (word >> 8) & 0xff;
                vga_pci.gr[0x12] = (word >> 16) & 0xff;
                vga_pci.gr[0x14] = (word >> 24) & 0xff;
                break;
            case 0x04:
                vga_pci.gr[0x01] = word & 0xff;
                vga_pci.gr[0x11] = (word >> 8) & 0xff;
                vga_pci.gr[0x13] = (word >> 16) & 0xff;
                vga_pci.gr[0x15] = (word >> 24) & 0xff;
                break;
            case 0x08:
                vga_pci.gr[0x20] = word & 0xff;
                vga_pci.gr[0x21] = (word >> 8) & 0xff;
                vga_pci.gr[0x22] = (word >> 16) & 0xff;
                vga_pci.gr[0x23] = (word >> 24) & 0xff;
                break;
            case 0x0c:
                vga_pci.gr[0x24] = word & 0xff;
                vga_pci.gr[0x25] = (word >> 8) & 0xff;
                vga_pci.gr[0x26] = (word >> 16) & 0xff;
                vga_pci.gr[0x27] = (word >> 24) & 0xff;
                break;
            case 0x10:
                vga_pci.gr[0x28] = word & 0xff;
                vga_pci.gr[0x29] = (word >> 8) & 0xff;
                vga_pci.gr[0x2a] = (word >> 16) & 0xff;
                vga_pci.gr[0x2b] = (word >> 24) & 0xff;
                break;
            case 0x14:
                vga_pci.gr[0x2c] = word & 0xff;
                vga_pci.gr[0x2d] = (word >> 8) & 0xff;
                vga_pci.gr[0x2e] = (word >> 16) & 0xff;
                vga_pci.gr[0x2f] = (word >> 24) & 0xff;
                break;
            case 0x18:
                vga_pci.gr[0x30] = word & 0xff;
                vga_pci.gr[0x32] = (word >> 16) & 0xff;
                vga_pci.gr[0x33] = (word >> 24) & 0xff;
                break;
            case 0x40:
                vga_pci.gr[0x31] = word & 0xff;
                if ((word & 0xff) == 0x02) {
                    clgd5440_start_bitblt();
                }
                break;
        }
        return 0;
    }
    fprintf(stderr, "CL-GD5440: Unhandled 32-bit write to 0x%08x\n", address);
    debug_info();
    exit(1);
    return 0;
}

/*
 ** Read a 16-bit word from RAM.
 */
int clgd5440_pci_mem_read_word(int address)
{
    if ((address & 0xff000000) == 0x81000000) { /* Linear memory */
        address &= 0x000ffffc;
        return vga_pci.ram[address] | (vga_pci.ram[address + 1] << 8) | (vga_pci.ram[address + 2] << 16) | (vga_pci.ram[address + 3] << 24);
    }
    fprintf(stderr, "CL-GD5440: Unhandled 16-bit read at address 0x%08x\n", address);
    debug_info();
    exit(1);
    return 0;
}

/*
 ** Subset of PCI header for OS detection (just vendor and device ID)
 */
static int clgd5440_pci_header[] = {
    0x00a01013, /* 0x1013 = Vendor: Cirrus Logic. 0x00a0 = Device: CLGD5440 */
    0x00000000,
    0x03000000, /* VGA */
    0x00000000,
    0x01000000, /* Video memory base address */
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
};

/*
 ** Read a 16-bit word from RAM.
 */
int clgd5440_pci_mem_read_dword(int address)
{
    if ((address & 0xc0000000) == 0xc0000000) {
        if ((address & 0x08000000) != 0) {  /* Slot closer to the power supply if I remember right */
            return clgd5440_pci_header[(address >> 2) & 0x0f];
        }
#if 0
        if ((address & 0x10080000) != 0) {
            
        }
        if ((address & 0x20080000) != 0) {
            
        }
#endif
        return 0xffffffff;  /* Nothing here */
    }
    if ((address & 0xffff8000) == 0x800b8000) {
        switch (address & 0xfc) {
            default:
                fprintf(stderr, "CL-GD5440: Unhandled byte read address 0x%08x\n", address);
                debug_info();
                exit(1);
            case 0x40:
                return vga_pci.gr[0x31];
        }
    }
    if (address < 0x000a0000 || address > 0x000bffff) {
        return -1;
    }
    address -= 0x000a0000;
    if (vga_pci.gr[0x0b] == 0x00) {
        address += (vga_pci.gr[0x09] & 0xfc) << 12;
        if (address >= 0x100000) {    /* 1 MB of RAM */
            fprintf(stderr, "CL-GD5440: Too big address 0x%08x\n", address);
            return -1;
        }
        return (vga_pci.ram[address + 1] << 8) | vga_pci.ram[address];
    }
    fprintf(stderr, "CL-GD5440: Unhandled byte read address 0x%08x\n", address);
    debug_info();
    exit(1);
    return 0;
};

/*
 ** Save the background under the cursor.
 */
void clgd5440_save_cursor(void)
{
    int source;
    int y;
    
    if ((vga_pci.sr[0x12] & 1) == 0)
        return;
    source = (vga_pci.cursor_y * 800 + vga_pci.cursor_x) * 2;
    for (y = 0; y < 32; y++) {
        source &= 0x0fffff;
        if (source + 64 > 0x100000)
            continue;
        memcpy(&vga_pci.cursor[y * 64], &vga_pci.ram[source], 64);
        source += 800 * 2;
    }
}

/*
 ** Restore the background under the cursor.
 */
void clgd5440_restore_cursor(void)
{
    int source;
    int y;

    if ((vga_pci.sr[0x12] & 1) == 0)
        return;
    source = (vga_pci.cursor_y * 800 + vga_pci.cursor_x) * 2;
    for (y = 0; y < 32; y++) {
        source &= 0x0fffff;
        if (source + 64 > 0x100000)
            continue;
        memcpy(&vga_pci.ram[source], &vga_pci.cursor[y * 64], 64);
        source += 800 * 2;
    }
}

/*
 ** Draw the cursor
 ** Currently only 32x32 pixels supported.
 */
void clgd5440_draw_cursor(void)
{
    int source;
    int target;
    int x;
    int y;
    int c;
    int color_1;
    int color_2;
    
    if ((vga_pci.sr[0x12] & 1) == 0)    /* Hardware cursor enabled? */
        return;
    color_1 = ((vga_pci.palette[0] & 0x3e) << 10) | ((vga_pci.palette[1] & 0x3f) << 5) | ((vga_pci.palette[2] & 0x3e) >> 1);
    color_2 = ((vga_pci.palette[765] & 0x3e) << 10) | ((vga_pci.palette[766] & 0x3f) << 5) | ((vga_pci.palette[767] & 0x3e) >> 1);
    for (y = 0; y < 32; y++) {
        source = 0xfc000 + (vga_pci.sr[0x13] * 256) + y * 4;
        target = ((vga_pci.cursor_y + y) * 800 + vga_pci.cursor_x) * 2;
        if (target + 64 > 0x100000)
            continue;
        for (x = 0; x < 4; x++) {
            for (c = 0x80; c; c >>= 1) {
                if ((vga_pci.ram[source + 128] & c) == 0) {
                    if ((vga_pci.ram[source] & c) != 0) {   /* XOR pixel */
                        vga_pci.ram[target + 0] = ~vga_pci.ram[target + 0];
                        vga_pci.ram[target + 1] = ~vga_pci.ram[target + 1];
                    }
                } else {
                    if ((vga_pci.ram[source] & c) == 0) {   /* Background color */
                        vga_pci.ram[target + 0] = color_1;
                        vga_pci.ram[target + 1] = color_1 >> 8;
                    } else {
                        vga_pci.ram[target + 0] = color_2;  /* Foreground color */
                        vga_pci.ram[target + 1] = color_2 >> 8;
                    }
                }
                target += 2;
            }
            source++;
        }
    }
}

/*
 ** Dump Cirrus Logic GD5440 registers
 ** (used before quitting in case of errors)
 */
void clgd5440_dump_registers(void)
{
    int c;
    
    fprintf(debug, "0x03c2= 0x%02x\n", vga_pci.mstatus);
    for (c = 0; c < 32; c++) {
        fprintf(debug, "SR%X= 0x%02x%c", c, vga_pci.sr[c], ((c & 3) == 3 ? '\n' : ' '));
    }
    for (c = 0; c < 32; c++) {
        fprintf(debug, "GR%X= 0x%02x%c", c, vga_pci.gr[c], ((c & 3) == 3 ? '\n' : ' '));
    }
    for (c = 0; c < 64; c++) {
        fprintf(debug, "CR%X= 0x%02x%c", c, vga_pci.cr[c], ((c & 3) == 3 ? '\n' : ' '));
    }
    for (c = 0; c < 32; c++) {
        fprintf(debug, "AR%X= 0x%02x%c", c, vga_pci.ar[c], ((c & 3) == 3 ? '\n' : ' '));
    }
}
