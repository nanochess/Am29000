/*
 ** G11 computer emulator (Am29000 processor)
 **
 ** by Oscar Toledo G.
 ** https://nanochess.org/
 **
 ** Creation date: Jul/17/2026.
 ** Revision date: Jul/18/2026. Added mapper for ROM and RAM. Reads disk image.
 ** Revision date: Jul/23/2026. Corrected LOADM and STOREM. Added EXTRACT.
 ** Revision date: Jul/25/2026. FC and BP registers are taken from the ALU.
 ** Revision date: Jul/26/2026. Solved bug in EXHWS instruction. Stable.
 ** Revision date: Jul/27/2026. Added SDL2 for displaying the screen.
 ** Revision date: Jul/28/2026. Added mouse and keyboard support. Added FDIV.
 **                             Implemented stub LPT1.
 ** Revision date: Jul/29/2026. Floppy disk is dropped onto window. Printer
 **                             file is created in documents folder.
 ** Revision date: Jul/30/2026. Moved processor to its own module.
 ** Revision date: Aug/10/2026. Added G11V2 support, and Cirrus Logic GD-5440
 **                             driver.
 ** Revision date: Aug/13/2026. Hard drive image now is marked as non-removable.
 */

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <wchar.h>
#include <SDL.h>
#include <winsock2.h>
#else
#include <SDL2/SDL.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "am29000.h"
#include "clgd5429.h"
#include "clgd5440.h"

#ifdef _WIN32
#else
#ifdef __APPLE__
#define PRINTER_FOLDER       "Documents/"
#else
#define PRINTER_FOLDER       ""
#endif
#endif

#ifdef _WIN32
wchar_t printer_filename[MAX_PATH];
#else
char printer_filename[4096];
#endif

#define APP_TITLE  "G11 emulator"

int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 600;
int quit;

void debug_info(void);

int mode;
int processor_running;
FILE *floppy;
int total_sectors;
FILE *floppy2;
FILE *printer_file;
FILE *debug;

//The window we'll be rendering to
SDL_Window* window = NULL;
//The surface contained by the window
SDL_Surface* screenSurface = NULL;
SDL_Surface* surface = NULL;

int lpt1_data;
int lpt1_strobe;

/*
 ** Reading from the ISA bus.
 */
uint32_t read_isa(uint32_t port)
{
    int c;
    int isa_port;
    
    if ((port & 0x80000000) != 0 && mode == 1) {
        c = clgd5440_pci_io_read_byte(port);
        c = (c & 0xff) << ((port & 3) * 8); /* Truly hideous code, put byte in the correct place in the bus */
        return c;
    }
    if (port == 0x80) { /* Crystal CS4280 if I remember right */
        return 0x00;
    }
    if (port == 0x84) {
        static int value;
        
        value ^= 0x20;
        return value;
    }
    if (port == 0x90) { /* WD33C93 status */
        return 0x80;
    }
    if (port == 0x94) { /* WD33C93 data */
        return 0x42;
    }
    if (port == 0xf0) { /* Keyboard data */
        return 0x00;
    }
    if (port == 0xf4) { /* Keyboard status */
        return 0x01;
    }
/*    fprintf(stderr, "Reading from port 0x%08x\n", port);*/
    isa_port = port / 4;
    if (isa_port == 0x3f4 || isa_port == 0x3f5) {   /* FDC port 0x3f4 / 0x3f5 */
        fprintf(stderr, "Unpatched floppy disk\n");
        debug_info();
        exit(1);
    }
    if (isa_port >= 0x03f8 && isa_port <= 0x03ff) { /* COM1 */
        return 0;
    }
    if (isa_port >= 0x02f8 && isa_port <= 0x02ff) { /* COM2 */
        return 0;
    }
    if (isa_port >= 0x03e8 && isa_port <= 0x03ef) { /* COM3 */
        return 0;
    }
    if (isa_port >= 0x02e8 && isa_port <= 0x02ef) { /* COM4 */
        return 0;
    }
    if (isa_port >= 0x03bc && isa_port <= 0x03bf) { /* LPT1 */
        if (isa_port == 0x3bc)
            return lpt1_data;
        if (isa_port == 0x3bd)
            return 0x80;    /* 0x80 = Ready, 0x00 = Busy */
        if (isa_port == 0x3be)
            return lpt1_strobe;
        return 0;
    }
    if (isa_port >= 0x0378 && isa_port <= 0x037b) { /* LPT2 */
        return 0;
    }
    if (isa_port >= 0x0278 && isa_port <= 0x027b) { /* LPT3 */
        return 0;
    }
    if ((isa_port & 0x0303) == 0x0203 || isa_port == 0x0303) {    /* Plug&Play */
        return 0xff;
    }
    if (isa_port == 0x0300) /* Probably Plug&Play */
        return 0xff;
    c = clgd5429_io_read_byte(isa_port);
    if (c == -1) {
        fprintf(stderr, "Unhandled port read 0x%02x\n", isa_port);
        debug_info();
        exit(1);
    }
    return c;
}

/*
 ** Writing to the ISA bus.
 */
void write_isaw(uint32_t port, uint32_t data)
{
    int isa_port;
    int c;
    
    if ((port & 0x80000000) != 0 && mode == 1) {
        c = clgd5440_pci_io_write_word(port & 0xffff, data);
    } else {
        isa_port = port / 4;
        c = clgd5429_io_write_word(isa_port, data & 0xffff);
    }
    if (c == -1) {
        fprintf(stderr, "Unhandled port write 0x%02x\n", port);
        debug_info();
        exit(1);
    }
}

/*
 ** Writing to the ISA bus.
 */
void write_isa(uint32_t port, uint32_t data)
{
    int isa_port;
    int c;
    
    if ((port & 0x80000000) != 0 && mode == 1) {
        c = clgd5440_pci_io_write_byte(port & 0xffff, data);
    } else {
        if (port == 0x0080 || port == 0x0084 || port == 0x0088) {
            /* Ignore */
            return;
        }
        if (port == 0x0090 || port == 0x0094) {
            /* Ignore */
            return;
        }
        if (port == 0xf0) { /* Keyboard write */
            /* !!! Ignore 0xed, 0x07, 0xf4 turn keyboard leds on */
            return;
        }
        if (port == 0x0000) {
            /* Ignore */
            return;
        }
        isa_port = port / 4;
        if (isa_port == 0x0a79) {   /* Plug&Play ISA */
            /* Ignore */
            return;
        }
        if (isa_port == 0x03f4 || isa_port == 0x03f5) {   /* FDC port 0x3f4 / 0x3f5 */
            fprintf(stderr, "Unpatched floppy disk\n");
            debug_info();
            exit(1);
        }
        if (isa_port >= 0x03f8 && isa_port <= 0x03ff) { /* COM1 */
            return;
        }
        if (isa_port >= 0x02f8 && isa_port <= 0x02ff) { /* COM2 */
            return;
        }
        if (isa_port >= 0x03e8 && isa_port <= 0x03ef) { /* COM3 */
            return;
        }
        if (isa_port >= 0x02e8 && isa_port <= 0x02ef) { /* COM4 */
            return;
        }
        if (isa_port >= 0x03bc && isa_port <= 0x03bf) { /* LPT1 */
            if (isa_port == 0x03bc)
                lpt1_data = data & 0xff;
            if (isa_port == 0x03be) {
                if ((lpt1_strobe & 1) != 0 && (data & 1) == 0)
                    fputc(lpt1_data, printer_file);
                lpt1_strobe = data & 0xff;
            }
            return;
        }
        if (isa_port >= 0x0378 && isa_port <= 0x037b) { /* LPT2 */
            return;
        }
        if (isa_port >= 0x0278 && isa_port <= 0x027b) { /* LPT3 */
            return;
        }
        if (isa_port == 0x0300 || isa_port == 0x0302 || isa_port == 0x0303) { /* Probably Plug&Play */
            return;
        }
        c = clgd5429_io_write_byte(isa_port, data & 0xff);
    }
    if (c == -1) {
        fprintf(stderr, "Unhandled port write 0x%02x\n", port);
        debug_info();
        exit(1);
    }
}

/*
 ** SCSI command parser
 */
void floppy_scsi(uint32_t unit, uint32_t subunit, uint32_t command, uint32_t command_length, uint32_t data, uint32_t data_length)
{
    /* Notice these are two separated strings automatically concatenated by the C language */
    unsigned char drive_info[] = "\x00\x80\x02\x02\x20\x00\x00\x00" "DisqueteSimulador SCSI-2v1.0";
    int c;
    int sector;
    int total;
    int count;
    unsigned char sector_data[512];
    
/*    fprintf(stderr, "floppy_scsi unit 0x%08x subunit 0x%08x commmand 0x%08x (0x%08x) data 0x%08x (0x%08x)\n", unit, subunit, command, command_length, data, data_length);*/
    if (debug != NULL)
        fprintf(debug, "floppy_scsi: executing command $%02x (unit %d)\n", read_byte(command), (int) unit);
    if (mode == 1) {
        if (unit != 0xfffffffeu && unit != 0x00000000) {
            regs[96] = 0x00420000;    /* Unit doesn't exist */
            regs[97] = 0;
            return;
        }
    }
    switch (read_byte(command)) {
        case 0x1a:    /* Mode sense */
            write_byte(data, 0x0b);
            if (unit == 0)
                write_byte(data + 1, 0x00);    /* Non-removable disk */
            else
                write_byte(data + 1, 0x81);    /* Medium type */
            write_byte(data + 2, 0);  /* 0x80 = WP Write Protected */
            write_byte(data + 3, 8);
            if (unit == 0)
                c = total_sectors;
            else
                c = 0x0b40;    /* Total blocks */
            write_byte(data + 4, c >> 24);
            write_byte(data + 5, c >> 16);
            write_byte(data + 6, c >> 8);
            write_byte(data + 7, c);
            c = 0x0200;    /* Block size */
            write_byte(data + 8, c >> 24);
            write_byte(data + 9, c >> 16);
            write_byte(data + 10, c >> 8);
            write_byte(data + 11, c);
            regs[96] = 0;    /* All good */
            regs[97] = 12;    /* 12 bytes returned */
            break;
        case 0x25:    /* Read Capacity */
            if (unit == 0)
                c = total_sectors - 1;
            else
                c = 0x0b3f;    /* Maximum block number */
            write_byte(data, c >> 24);
            write_byte(data + 1, c >> 16);
            write_byte(data + 2, c >> 8);
            write_byte(data + 3, c);
            c = 512;    /* Block size */
            write_byte(data + 4, c >> 24);
            write_byte(data + 5, c >> 16);
            write_byte(data + 6, c >> 8);
            write_byte(data + 7, c);
            regs[96] = 0;    /* All good */
            regs[97] = 8;    /* 8 bytes returned */
            break;
        case 0x12:    /* Inquiry */
            for (c = 0; c < 36; c++) {
                write_byte(data + c, drive_info[c]);
            }
            regs[96] = 0;    /* All good */
            regs[97] = 36;    /* 36 bytes returned */
            break;
        case 0x00:    /* Test ready */
            regs[96] = 0;    /* All good */
            regs[97] = 0;
            break;
        case 0x03:    /* Request sense */
            c = 0x70000000;
            write_byte(data, c >> 24);
            write_byte(data + 1, c >> 16);
            write_byte(data + 2, c >> 8);
            write_byte(data + 3, c);
            c = 0x0000000a;
            write_byte(data + 4, c >> 24);
            write_byte(data + 5, c >> 16);
            write_byte(data + 6, c >> 8);
            write_byte(data + 7, c);
            c = 0x00000000;
            write_byte(data + 8, c >> 24);
            write_byte(data + 9, c >> 16);
            write_byte(data + 10, c >> 8);
            write_byte(data + 11, c);
            if (mode == 1) {
                if (unit == 0xfffffffeu && floppy2 == NULL) {
                    c = 0x3a00 << 16;   /* Medium not present */
                } else {
                    c = 0x0000 << 16;   /* Error code */
                }
            } else {
                if (unit == 0xffffffffu && floppy2 == NULL) {
                    c = 0x3a00 << 16;   /* Medium not present */
                } else {
                    c = 0x0000 << 16;   /* Error code */
                }
            }
            write_byte(data + 12, c >> 24);
            write_byte(data + 13, c >> 16);
            write_byte(data + 14, c >> 8);
            write_byte(data + 15, c);
            write_byte(data + 16, 0);
            write_byte(data + 17, 0);
            regs[96] = 0;    /* All good */
            regs[97] = 18;    /* 18 bytes returned */
            break;
        case 0x28:    /* Read (10) */
            if (mode == 1) {
                if (unit == 0xfffffffeu && floppy2 == NULL) {
                    regs[96] = 2;
                    regs[97] = 0;
                    break;
                }
            } else {
                if (unit == 0xffffffffu && floppy2 == NULL) {
                    regs[96] = 2;
                    regs[97] = 0;
                    break;
                }
            }
            sector = (read_byte(command + 3) << 16) | (read_byte(command + 4) << 8) | read_byte(command + 5);
            total = (read_byte(command + 7) << 8) | read_byte(command + 8);
            fprintf(stderr, "Reading sector %d, length %d (addr=0x%08x)\n", sector, total, data);
            for (count = 0; count < total; count++) {
                if (mode == 1) {
                    fseek((unit == 0 ? floppy : floppy2), sector * 512, SEEK_SET);
                    fread(sector_data, 1, 512, (unit == 0 ? floppy : floppy2));
                } else {
                    fseek((unit == 0xfffffffeu ? floppy : floppy2), sector * 512, SEEK_SET);
                    fread(sector_data, 1, 512, (unit == 0xfffffffeu ? floppy : floppy2));
                }
                for (c = 0; c < 512; c++) {
                    write_byte(data, sector_data[c]);
                    data++;
                }
                sector++;
            }
            regs[96] = 0;
            regs[97] = total * 512;
            break;
        case 0x2a:    /* Write (10) */
            if (mode == 1) {
                if (unit == 0xfffffffeu && floppy2 == NULL) {
                    regs[96] = 2;
                    regs[97] = 0;
                    break;
                }
            } else {
                if (unit == 0xffffffffu && floppy2 == NULL) {
                    regs[96] = 2;
                    regs[97] = 0;
                    break;
                }
            }
            sector = (read_byte(command + 3) << 16) | (read_byte(command + 4) << 8) | read_byte(command + 5);
            total = (read_byte(command + 7) << 8) | read_byte(command + 8);
            fprintf(stderr, "Writing sector %d, length %d (addr=0x%08x)\n", sector, total, data);
            for (count = 0; count < total; count++) {
                for (c = 0; c < 512; c++) {
                    sector_data[c] = read_byte(data);
                    data++;
                }
                if (mode == 1) {
                    fseek((unit == 0 ? floppy : floppy2), sector * 512, SEEK_SET);
                    fwrite(sector_data, 1, 512, (unit == 0 ? floppy : floppy2));
                } else {
                    fseek((unit == 0xfffffffeu ? floppy : floppy2), sector * 512, SEEK_SET);
                    fwrite(sector_data, 1, 512, (unit == 0xfffffffeu ? floppy : floppy2));
                }
                sector++;
            }
            regs[96] = 0;
            regs[97] = total * 512;
            break;
        default:    /* Unsupported commannd */
            regs[96] = 2;    /* Unsupported command */
            regs[97] = 0;
            break;
    }
}

/*
 ** Generate debug information.
 */
void debug_info(void)
{
    uint32_t c;
    uint32_t d;
    int e;
    FILE *output;
    uint32_t instruction;
    char string[256];
    
    if (debug == NULL)
        debug = stderr;
    fprintf(debug, "gr1=0x%08x\n", regs[1]);
    for (c = 64; c < 128; c++) {
        fprintf(debug, "gr%d=0x%08x%c", c, regs[c], ((c & 3) == 3 ? '\n' : ' '));
    }
    for (c = 128; c < 256; c++) {
        fprintf(debug, "lr%d=0x%08x%c", c - 128, regs[REG_AA(c)], ((c & 3) == 3 ? '\n' : ' '));
    }
    if (mode == 1)
        clgd5440_dump_registers();
    else
        clgd5429_dump_registers();
    if (debug != stderr)
        fclose(debug);
    debug = NULL;
    if (mode == 1) {
        output = fopen("rom.txt", "w");
        for (c = 0x00000000; c <= 0x00100000; c += 4) {
            instruction = read_word(c);
            disassemble(c, instruction, string);
            fprintf(output, "0x%08X: 0x%08x  %s\n", c, instruction, string);
        }
        fclose(output);
    }
    output = fopen("ram.txt", "w");
    for (c = 0xbff80000; c <= 0xc0000000; c += 16) {
        fprintf(output, "%08x: %08x %08x %08x %08x  ", c, read_word(c), read_word(c + 4), read_word(c + 8), read_word(c + 12));
        for (d = 0; d < 16; d++) {
            e = read_byte(c + d);
            if (e < 0x20)
                e = '.';
            else if (e > 0x7f)
                e = '.';
            fputc(e, output);
        }
        fputc('\n', output);
    }
    fclose(output);
    output = fopen("disassembly.txt", "w");
    for (c = 0xbff80000; c <= 0xc0000000; c += 4) {
        instruction = read_word(c);
        disassemble(c, instruction, string);
        fprintf(output, "0x%08X: 0x%08x  %s\n", c, instruction, string);
    }
    fclose(output);
    fflush(floppy2);
    fclose(floppy2);
    fflush(floppy);
    fclose(floppy);
}

int x_left;
int y_top;
int x_width;
int y_height;
/*
 ** Update screen
 */
void update_screen(void)
{
    SDL_Rect dest;
    double ratio_x;
    double ratio_y;
    double ratio;
    static int previous_x_size = -1;
    static int previous_y_size = -1;

    ratio_x = SCREEN_WIDTH / 800.0;
    ratio_y = SCREEN_HEIGHT / 600.0;
    ratio = (ratio_x < ratio_y) ? ratio_x : ratio_y;
    // 256.0 for square aspect
    // 288.0 is how it looks in a real TV
    x_width = (int) (ratio * 800.0);
    y_height = (int) (ratio * 600.0);
    x_left = (SCREEN_WIDTH - x_width) / 2;
    y_top = (SCREEN_HEIGHT - y_height) / 2;
    
    // Fill the surface with border color, but only the non-updated parts to optimize it ;)
    if (previous_x_size != SCREEN_WIDTH || previous_y_size != SCREEN_HEIGHT) {
        dest.w = SCREEN_WIDTH;
        dest.h = y_top;
        dest.x = 0;
        dest.y = 0;
        SDL_FillRect(screenSurface, &dest, SDL_MapRGB(screenSurface->format, 0, 0, 0));
        dest.w = x_left;
        dest.h = y_height;
        dest.x = 0;
        dest.y = y_top;
        SDL_FillRect(screenSurface, &dest, SDL_MapRGB(screenSurface->format, 0, 0, 0));
        dest.w = SCREEN_WIDTH - (x_left + x_width);
        dest.h = y_height;
        dest.x = x_left + x_width;
        dest.y = y_top;
        SDL_FillRect(screenSurface, &dest, SDL_MapRGB(screenSurface->format, 0, 0, 0));
        dest.w = SCREEN_WIDTH;
        dest.h = SCREEN_HEIGHT - (y_top + y_height);
        dest.x = 0;
        dest.y = y_top + y_height;
        SDL_FillRect(screenSurface, &dest, SDL_MapRGB(screenSurface->format, 0, 0, 0));
        previous_x_size = SCREEN_WIDTH;
        previous_y_size = SCREEN_HEIGHT;
    }
    
    // Blit the screen
    dest.w = x_width;
    dest.h = y_height;
    dest.x = x_left;
    dest.y = y_top;
    SDL_BlitScaled(surface, NULL, screenSurface, &dest);
    SDL_UpdateWindowSurface(window);
}

/*
 ** Add a key to buffer
 */
void add_keyboard(int code)
{
    uint32_t start;
    uint32_t end;
    uint32_t writer;
    
    start = read_word(0xbffffa00);
    end = read_word(0xbffffa04);
    writer = read_word(0xbffffa08);
    write_word(writer, code);
    writer += 4;
    if (writer == end)
        writer = start;
    write_word(0xbffffa08, writer);
    
    /*
     ** Patch keyboard code
     */
    if (mode == 0) {
        write_word(0x8002be28, 0x16048789);   /* LOAD 0,0x04,lr7,lr9 */
        write_word(0x8002be50, 0xa0000074);   /* Jump to 8002c020 */
        write_word(0x8002be54, 0x03008c00);   /* Shift keys status is zero */
    } else if (mode == 1) {
        rom[0x00066870 / 4] = 0x16048783;   /* LOAD 0,0x04,lr7,lr3 */
        rom[0x00066894 / 4] = 0x03008e00;   /* Scancode to zero */
        rom[0x00066898 / 4] = 0xa0000074;   /* Jump to 00066a68 */
        rom[0x0006689C / 4] = 0x03008c00;   /* Shift keys status is zero */
    }
}

/*
 ** Do something (UI interface)
 */
void do_something(void)
{
    SDL_Event e;
    
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            quit = 1;
        } else if (e.type == SDL_WINDOWEVENT) {
            switch (e.window.event) {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    SCREEN_WIDTH = e.window.data1;
                    SCREEN_HEIGHT = e.window.data2;
                    // Doesn't work if you forget the following line
                    screenSurface = SDL_GetWindowSurface( window );
                    break;
                case SDL_WINDOWEVENT_RESTORED:
                case SDL_WINDOWEVENT_MAXIMIZED:
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    update_screen();
                    break;
            }
            
        /*
        ** The actual mouse controller is contained in 0x80011700 - 0x80011cff
        ** but instead of writing a UART driver, it changes the same registers.
        */
        } else if (e.type == SDL_MOUSEMOTION) {
            int x;
            int y;
            
            x = e.motion.x - x_left;
            x = (x * 800) / x_width;
            y = e.motion.y - y_top;
            y = (y * 600) / y_height;
            x -= 16;    /* The cursor coordinates are given for the top-left corner */
            y -= 16;    /* So the cursor bitmap should point to the middle */
            if (x < 0)
                x = 0;
            if (y < 0)
                y = 0;
            if (x > 800 - 32)
                x = 800 - 32;
            if (y > 600 - 32)
                y = 600 - 32;
            regs[92] = x;
            regs[91] = y;
            write_word(0xbffffaec, 0x00010000); /* Patch the network services */
        } else if (e.type == SDL_MOUSEBUTTONDOWN) {
            if (e.button.button == SDL_BUTTON_LEFT) {
                regs[90] |= 0x04;
            } else if (e.button.button == SDL_BUTTON_MIDDLE) {
                regs[90] |= 0x02;
            } else if (e.button.button == SDL_BUTTON_RIGHT) {
                regs[90] |= 0x01;
            }
        } else if (e.type == SDL_MOUSEBUTTONUP) {
            if (e.button.button == SDL_BUTTON_LEFT) {
                regs[90] &= ~0x04;
            } else if (e.button.button == SDL_BUTTON_MIDDLE) {
                regs[90] &= ~0x02;
            } else if (e.button.button == SDL_BUTTON_RIGHT) {
                regs[90] &= ~0x01;
            }
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_BACKSPACE:
                    add_keyboard(0x08);
                    break;
                case SDLK_RETURN:
                    add_keyboard(0x0d);
                    break;
                case SDLK_ESCAPE:
                    add_keyboard(0x1b);
                    break;
                case SDLK_F1:
                    add_keyboard(0x01);
                    break;
                case SDLK_F2:
                    add_keyboard(0x02);
                    break;
                case SDLK_F3:
                    add_keyboard(0x03);
                    break;
                case SDLK_F4:
                    add_keyboard(0x04);
                    break;
                case SDLK_UP:
                    add_keyboard(0x18);
                    break;
                case SDLK_DOWN:
                    add_keyboard(0x12);
                    break;
                case SDLK_LEFT:
                    add_keyboard(0x14);
                    break;
                case SDLK_RIGHT:
                    add_keyboard(0x16);
                    break;
            }
        } else if (e.type == SDL_TEXTINPUT) {   /* Pure text */
            char *p;
            
            p = &e.text.text[0];
            while (*p) {
                if ((p[0] & 0xf8) == 0xf0 && (p[1] & 0xc0) == 0x80 && (p[2] & 0xc0) == 0x80 && (p[3] & 0xc0) == 0x80) {
                    add_keyboard('?');
                    p += 4; /* Ignored */
                } else if ((p[0] & 0xf0) == 0xe0 && (p[1] & 0xc0) == 0x80 && (p[2] & 0xc0) == 0x80) {
                    add_keyboard(((p[0] & 0x0f) << 12) | ((p[1] & 0x3f) << 6) | (p[2] & 0x3f));
                    p += 3;
                } else if ((p[0] & 0xe0) == 0xc0 && (p[1] & 0x80) == 0x80) {
                    add_keyboard(((p[0] & 0x1f) << 6) | (p[1] & 0x3f));
                    p += 2;
                } else {
                    add_keyboard(p[0] & 0xff);
                    p += 1;
                }
            }
        } else if (e.type == SDL_DROPFILE) {
            if (floppy == NULL) {
                unsigned char buffer[4];
                uint32_t c;
                uint32_t d;
                uint32_t instruction;
                
                floppy = fopen(e.drop.file, "rb+");
                fseek(floppy, 0, SEEK_END);
                c = (uint32_t) ftell(floppy);
                if (c > 1474560) {  /* It is bigger than 1.44mb? Then G11V2 mode */
                    total_sectors = c / 512;
                    memcpy(rom, rom_1999, sizeof(rom_1999));
                    memset(memory, 0x00, sizeof(memory));   /* Avoids a bug */
                    if (regs[92] > 640 && regs[91] > 480) {   /* Drop in bottom-right corner gets extra RAM */
                        pc1 = 0x00040020;
                        rom[0x00040028 / 4] = 0x70406161;
                    } else {
                        pc1 = 0x00040000;
                    }
                    pc0 = pc1 + 4;
                    endianness = 0; /* Shouldn't be done here */
                    special[3] = 0x00000004u;
                    write_word(0xbfffffd8, RAM_SIZE / 1024); /* Kilobytes of RAM */
                    write_word(0xbfffffdc, 0x00); /* Boot drive (0x40 for A, 0-7 for C-I) */
                    write_word(0xbfffffe0, 12); /* 12 mhz ??? */
                    /* Avoid traps in emulation code */
                    rom[0x000400b0 / 4] = 0x03005f40;   /* CONST gr95,0x0040 */
                    rom[0x000400b4 / 4] = 0x02005f04;   /* CONSTH gr95,0x0004 */
                    /* Patch the SCSI routine */
                    rom[0x0004a008 / 4] = 0xfc000280;   /* Call the emulator */
                    rom[0x0004a848 / 4] = 0x03006001;   /* Initialization successful */
                    /* Patch the network services */
                    for (c = 0; c < 24; c++) {
                        rom[(0x00010000 + c * 4) / 4] = 0x00020000 + c * 8; /* Address table */
                        rom[(0x00020000 + c * 8) / 4] = 0xfc000080 + ((c + 16) << 8);   /* Emulator instruction */
                        rom[(0x00020004 + c * 8) / 4] = 0x70400101;   /* Emulator instruction */
                    }
                    mode = 1;
                } else {    /* G11V1 mode */
                    memset(rom, 0xff, sizeof(rom));
                    memset(memory, 0xff, sizeof(memory));
                    
                    /* Load boot code */
                    d = 0xbfff0000;
                    fseek(floppy, 0, SEEK_SET);
                    for (c = 0; c < 9216; c += 4) {
                        fread(buffer, 1, 4, floppy);
                        instruction = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
                        write_word(d, instruction);
                        d += 4;
                    }
                    pc1 = 0xbfff0000;
                    pc0 = pc1 + 4;
                    endianness = 3; /* Shouldn't be done here */
                    special[3] = 0x00000000u;
                    
                    write_word(0xbfffffd8, RAM_SIZE / 1024); /* Kilobytes of RAM */
                    write_word(0xbfffffdc, 0x40); /* Boot drive (0x40 for A, 0-7 for C-I) */
                    write_word(0xbfffffe0, 12); /* 12 mhz ??? */
                    /* Patch RAM with boot loader services */
                    write_word(0xbfffffec, 0x00002000);
                    
                    /* Text drawing routine, gr64=x, gr65=y, gr66=text */
                    rom[0x00002018 / 4] = 0xc0000080;   /* JMPI lr0 */
                    rom[0x0000201c / 4] = 0x70406161;   /* NOP */
                    /* Read disk, gr111=track and head, gr113=target address, gr77=bytes */
                    rom[0x00002020 / 4] = 0xfc000180;   /* Use a non-implemented instruction */
                    rom[0x00002024 / 4] = 0x70406161;   /* NOP */
                    /* Box drawing routine, gr64=x, gr65=y, gr66=w, gr67=h, gr68=title */
                    rom[0x00002050 / 4] = 0xc0000080;   /* JMPI lr0 */
                    rom[0x00002054 / 4] = 0x70406161;   /* NOP */
                    /* Icon drawing routine, gr64=x, gr65=y, gr66=icon */
                    rom[0x00002058 / 4] = 0xc0000080;   /* JMPI lr0 */
                    rom[0x0000205c / 4] = 0x70406161;   /* NOP */
                    /* Text drawing routine, gr64=x, gr65=y, gr66=text */
                    rom[0x00002078 / 4] = 0xc0000080;   /* JMPI lr0 */
                    rom[0x0000207c / 4] = 0x70406161;   /* NOP */
                    mode = 0;
                }
                processor_running = 1;
            } else {
                if (floppy2 != NULL)
                    fclose(floppy2);
                floppy2 = fopen(e.drop.file, "rb+");
            }
        }
    }
}

/*
 ** Main program.
 */
#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hThisInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpszArgument,
                   int nFunsterStil)
#else
int main(int argc, const char *argv[])
#endif
{
    uint32_t c;
    uint32_t d;
    uint16_t *image;
    static int next_file = 0;

#ifdef _WIN32
    WSADATA data;
    wchar_t *ap;
    
    SHGetFolderPathW(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, printer_filename);
    if (printer_filename[wcslen(printer_filename) - 1] != '\\')
        wcscat(printer_filename, L"\\");
    ap = printer_filename + wcslen(printer_filename);
    do {
        wsprintf(ap, L"printer%03d.txt", ++next_file);
        printer_file = _wfopen(printer_filename, L"rb");
        if (printer_file == NULL)
            break;
    } while (1) ;
    printer_file = _wfopen(printer_filename, L"w");
    WSAStartup(MAKEWORD(2, 2), &data);
#else
    char *ap;
    
    snprintf(printer_filename, sizeof(printer_filename), "%s", getenv("HOME"));
    if (printer_filename[strlen(printer_filename) - 1] != '/')
        strcat(printer_filename, "/");
    ap = printer_filename + strlen(printer_filename);
    do {
        sprintf(ap, PRINTER_FOLDER "printer%03d.txt", ++next_file);
        printer_file = fopen(printer_filename, "rb");
        if (printer_file == NULL)
            break;
    } while (1) ;
    printer_file = fopen(printer_filename, "w");
#endif
    {
        image = (uint16_t *) vga.ram;
        for (c = 0; c < 600; c++) {
            for (d = 0; d < 800; d++)
                *image++ = c * 32 / 600;
        }
    }
    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0 ) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 0;
    }

    //Create window
    window = SDL_CreateWindow(APP_TITLE,
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if( window == NULL ) {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
    }
    //Get window surface
    screenSurface = SDL_GetWindowSurface( window );

    // Create surface using masks for 16-bit RGB. Not sure if this works always.
    surface = SDL_CreateRGBSurface(0, 800, 600, 16, 0xf800, 0x07e0, 0x001f, 0);
    if(surface == NULL) {
        fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
        exit(1);
    }
    SDL_ShowCursor(SDL_DISABLE);

#if 0
    fprintf(stderr, "Computer emulator v0.1\n");
    fprintf(stderr, "by Oscar Toledo G. https://nanochess.org/\n");
    fprintf(stderr, "\n");
    c = 1;
    if (c < argc && argv[c][0] == '-' && tolower(argv[c][1]) == 'd') {
        debug = fopen("debug.txt", "w");
        c++;
    } else {
        debug = NULL;
    }
    if (c >= argc) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "\temulator [-d] floppy.img\n");
        exit(1);
    }
    floppy = fopen(argv[c], "rb+");
#else
    debug = NULL;
/*   debug = fopen("/Users/oscartoledo/Documents/debug.txt", "w");*/
#endif
    processor_running = 0;
    count = 0;
    while (!quit) {
        if (processor_running) {
            if (mode == 1) {
                do {
                    am29000_emulate();
                } while (count % 200000 != 0) ; /* 12 mhz */
            } else {
                do {
                    am29000_emulate();
                } while (count % 100000 != 0) ; /* 6 mhz */
            }
        }
        SDL_Delay(1000 / 60);
        if (mode == 1) {
            clgd5440_save_cursor();
            clgd5440_draw_cursor();
            if (SDL_MUSTLOCK(surface))
                SDL_LockSurface(surface);
            image = surface->pixels;
            for (c = 0; c < 600; c++) {
                memcpy(image + c * 800, &vga_pci.ram[c * 800 * sizeof(uint16_t)], 800 * sizeof(uint16_t));
            }
            if (SDL_MUSTLOCK(surface))
                SDL_UnlockSurface(surface);
            clgd5440_restore_cursor();
        } else {
            clgd5429_save_cursor();
            clgd5429_draw_cursor();
            if (SDL_MUSTLOCK(surface))
                SDL_LockSurface(surface);
            image = surface->pixels;
            for (c = 0; c < 600; c++) {
                memcpy(image + c * 800, &vga.ram[c * 800 * sizeof(uint16_t)], 800 * sizeof(uint16_t));
            }
            if (SDL_MUSTLOCK(surface))
                SDL_UnlockSurface(surface);
            clgd5429_restore_cursor();
        }
        update_screen();
        do_something();
    }
    if (floppy2 != NULL)
        fclose(floppy2);
    if (floppy != NULL)
        fclose(floppy);
    if (printer_file != NULL)
        fclose(printer_file);

    SDL_FreeSurface(surface);
    
    //Destroy window
    SDL_DestroyWindow( window );
    
    //Quit SDL subsystems
    SDL_Quit();
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
