//
// Am29000 emulator
//
// by Oscar Toledo G.
// All rights reserved.
// https://nanochess.org/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Creation date: Aug/05/2026.
//


//
// Notice the use of >>> 0 to create an unsigned 32-bit number.
//
// Programming guide:
// o Read registers as you want (for speed), but write always with >>> 0.
//

const debug = 0;

var current_instruction = 0;
var last_instruction = new Array(1024);

var rom = new Uint32Array(131072 / 4);
var memory = new Uint32Array(524288 / 4);
var regs = new Uint32Array(256);
var special = new Uint32Array(256);

var interval;

var pc0;
var pc1;
var pc2;

var count = 0;
var endianness;

var first_time = 1;

// Get hexadecimal 8 digits
function hex(value)
{
    return ("0000000" + value.toString(16)).substr(-8);
}

// Debug information
function debug_info()
{
    var c;
    var d;
    var p;
    
    console.debug("gr1=0x" + regs[1].toString(16) + "\n");
    for (c = 64; c < 128; c += 4) {
        console.debug("gr" + c + "=0x" + hex(regs[c]) + " " +
                      "gr" + (c + 1) + "=0x" + hex(regs[c + 1]) + " " +
                      "gr" + (c + 2) + "=0x" + hex(regs[c + 2]) + " " +
                      "gr" + (c + 3) + "=0x" + hex(regs[c + 3]) + "\n");
    }
    for (c = 128; c < 256; c += 4) {
        d = ((c + regs[1] / 4) | 0x80) & 0xff;
        console.debug("lr" + (c - 128 + 0) + "=0x" + hex(regs[d]) + " " +
                      "lr" + (c - 128 + 1) + "=0x" + hex(regs[((d + 1) & 0x7f) | 0x80]) + " " +
                      "lr" + (c - 128 + 2) + "=0x" + hex(regs[((d + 2) & 0x7f) | 0x80]) + " " +
                      "lr" + (c - 128 + 3) + "=0x" + hex(regs[((d + 3) & 0x7f) | 0x80]) + "\n");
    }
    document.writeln("<pre>");
    for (c = 0xbff80000; c <= 0xc0000000; c += 16) {
        p = hex(c) + ": " + hex(read_word(c)) + " " + hex(read_word(c + 4)) + " " + hex(read_word(c + 8)) + " " + hex(read_word(c + 12)) + " ";
        for (d = 0; d < 16; d++) {
            e = read_byte(c + d);
            if (e < 0x20 || e > 0x7f)
                e = 46;
            p += String.fromCharCode(e);
        }
        document.writeln(p);
    }
    document.writeln("</pre>");
}

// Read a word.
function read_word(addr)
{
    return (addr & 0x80000000) ? memory[(addr >> 2) & 0x1ffff] : rom[(addr >> 2) & 0x7fff];
}

// Write a word.
function write_word(addr, data)
{
    if (addr & 0x80000000)
        memory[(addr >> 2) & 0x1ffff] = data >>> 0;
}

// Read a byte.
function read_byte(addr)
{
    return (read_word(addr) >> (8 * ((addr ^ endianness) & 3))) & 0xff;
}

// Write a byte.
function write_byte(addr, byte)
{
    var word;
    
    byte &= 0xff;
    word = read_word(addr);
    word &= ~(0x000000ff << (8 * ((addr ^ endianness) & 3)));
    word |= byte << (8 * ((addr ^ endianness) & 3));
    write_word(addr, word);
}

var lz = [
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
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];

// Floppy disk drives
 
var floppy1 = new Uint8Array(80 * 18432);

disk_image = window.atob(disk_image);

for (c = 0; c < disk_image.length; c++)
    floppy1[c] = disk_image.charCodeAt(c);
for (; c < 80 * 18432; c++)
    floppy1[c] = 0xfc;

var floppy2 = new Uint8Array(80 * 18432);

empty_disk_image = window.atob(empty_disk_image);

for (c = 0; c < empty_disk_image.length; c++)
    floppy2[c] = empty_disk_image.charCodeAt(c);
for (; c < 80 * 18432; c++)
    floppy2[c] = 0xfc;

var drive_info = [
    0x00, 0x80, 0x02, 0x02, 0x20, 0x00, 0x00, 0x00,
    0x44, 0x69, 0x73, 0x71, 0x75, 0x65, 0x74, 0x65,
    0x53, 0x69, 0x6d, 0x75, 0x6c, 0x61, 0x64, 0x6f,
    0x72, 0x20, 0x53, 0x43, 0x53, 0x49, 0x2d, 0x32,
    0x76, 0x31, 0x2e, 0x30,
];

var isa_value = 0x00;
var lpt1_data;
var lpt1_strobe;

// Reading from the ISA bus.
function read_isa(port)
{
    var c;
    var isa_port;
    
    if (port == 0x80) { // Crystal CS4280 if I remember right 
        return 0x00;
    }
    if (port == 0x84) {
        isa_value ^= 0x20;
        return isa_value;
    }
    if (port == 0x90) {   // I forgot why this is done XD 
        return 0x80;
    }
    if (port == 0x94) {
        return 0x42;    // No idea 
    }
    if (port == 0xf4) { // Keyboard status 
        return 0x00;
    }
    isa_port = port / 4;
    if (isa_port == 0x3f4 || isa_port == 0x3f5)   // FDC port 0x3f4 / 0x3f5 
        throw "Unpatched floppy disk\n";
    if (isa_port >= 0x03f8 && isa_port <= 0x03ff) { // COM1 
        return 0;
    }
    if (isa_port >= 0x02f8 && isa_port <= 0x02ff) { // COM2 
        return 0;
    }
    if (isa_port >= 0x03e8 && isa_port <= 0x03ef) { // COM3 
        return 0;
    }
    if (isa_port >= 0x02e8 && isa_port <= 0x02ef) { // COM4 
        return 0;
    }
    if (isa_port >= 0x03bc && isa_port <= 0x03bf) { // LPT1 
        if (isa_port == 0x3bc)
            return lpt1_data;
        if (isa_port == 0x3bd)
            return 0x80;    // 0x80 = Ready, 0x00 = Busy 
        if (isa_port == 0x3be)
            return lpt1_strobe;
        return 0;
    }
    if (isa_port >= 0x0378 && isa_port <= 0x037b) { // LPT2 
        return 0;
    }
    if (isa_port >= 0x0278 && isa_port <= 0x027b) { // LPT3 
        return 0;
    }
    c = clgd5429.io_read_byte(isa_port);
    if (c == -1) {
        throw "Unhandled port read 0x" + isa_port.toString(16) + "\n";
    }
    return c;
}

// Writing to the ISA bus.
function write_isaw(port, data)
{
    var isa_port;
    var c;
    
    isa_port = port / 4;
    c = clgd5429.io_write_word(isa_port, data & 0xffff);
    if (c == -1) {
        throw "Unhandled port write 0x" + isa_port.toString(16) + "\n";
    }
}

// Writing to the ISA bus.
function write_isa(port, data)
{
    var isa_port;
    var c;
    
    if (port == 0x0080 || port == 0x0084 || port == 0x0088) {
        // Ignore 
        return;
    }
    if (port == 0x0090 || port == 0x0094) {
        // Ignore 
        return;
    }
    if (port == 0xf0) { // Keyboard write 
        // !!! Ignore 0xed, 0x07, 0xf4 turn keyboard leds on 
        return;
    }
    if (port == 0x0000) {
        // Ignore 
        return;
    }
    isa_port = port / 4;
    if (isa_port == 0x03f4 || isa_port == 0x03f5) {   // FDC port 0x3f4 / 0x3f5
        throw "Unpatched floppy disk\n";
    }
    if (isa_port >= 0x03f8 && isa_port <= 0x03ff) { // COM1 
        return;
    }
    if (isa_port >= 0x02f8 && isa_port <= 0x02ff) { // COM2 
        return;
    }
    if (isa_port >= 0x03e8 && isa_port <= 0x03ef) { // COM3 
        return;
    }
    if (isa_port >= 0x02e8 && isa_port <= 0x02ef) { // COM4 
        return;
    }
    if (isa_port >= 0x03bc && isa_port <= 0x03bf) { // LPT1 
        if (isa_port == 0x03bc)
            lpt1_data = data & 0xff;
        if (isa_port == 0x03be) {
            if ((lpt1_strobe & 1) != 0 && (data & 1) == 0) {
                // fputc(lpt1_data, printer_file);
            }
            lpt1_strobe = data & 0xff;
        }
        return;
    }
    if (isa_port >= 0x0378 && isa_port <= 0x037b) { // LPT2 
        return;
    }
    if (isa_port >= 0x0278 && isa_port <= 0x027b) { // LPT3 
        return;
    }
    c = clgd5429.io_write_byte(isa_port, data & 0xff);
    if (c == -1) {
        throw "Unhandled port write 0x" + isa_port.toString(16) + "\n";
    }
}

// SCSI command parser.
function floppy_scsi(unit, subunit, command, command_length, data, data_length)
{
    var c;
    var pos;
    var sector;
    var total;
    var count;
    
    console.debug("floppy_scsi: executing command 0x" + read_byte(command).toString(16) + "\n");
    switch (read_byte(command)) {
        case 0x1a:    // Mode sense 
            write_byte(data, 0x0b);
            write_byte(data + 1, 0x81);    // Medium type 
            write_byte(data + 2, 0);  // 0x80 = WP Write Protected 
            write_byte(data + 3, 8);
            c = 0x0b40;    // Total blocks 
            write_byte(data + 4, c >> 24);
            write_byte(data + 5, c >> 16);
            write_byte(data + 6, c >> 8);
            write_byte(data + 7, c);
            c = 0x0200;    // Block size 
            write_byte(data + 8, c >> 24);
            write_byte(data + 9, c >> 16);
            write_byte(data + 10, c >> 8);
            write_byte(data + 11, c);
            regs[96] = 0;    // All good 
            regs[97] = 12;    // 12 bytes returned 
            break;
        case 0x25:    // Read Capacity 
            c = 0x0b3f;    // Maximum block number 
            write_byte(data, c >> 24);
            write_byte(data + 1, c >> 16);
            write_byte(data + 2, c >> 8);
            write_byte(data + 3, c);
            c = 512;    // Block size 
            write_byte(data + 4, c >> 24);
            write_byte(data + 5, c >> 16);
            write_byte(data + 6, c >> 8);
            write_byte(data + 7, c);
            regs[96] = 0;    // All good 
            regs[97] = 8;    // 8 bytes returned 
            break;
        case 0x12:    // Inquiry 
            for (c = 0; c < 36; c++) {
                write_byte(data + c, drive_info[c]);
            }
            regs[96] = 0;    // All good 
            regs[97] = 36;    // 36 bytes returned 
            break;
        case 0x00:    // Test ready 
            regs[96] = 0;    // All good 
            regs[97] = 0;
            break;
        case 0x03:    // Request sense 
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
            c = 0x0000 << 16;   // Error code
            write_byte(data + 12, c >> 24);
            write_byte(data + 13, c >> 16);
            write_byte(data + 14, c >> 8);
            write_byte(data + 15, c);
            write_byte(data + 16, 0);
            write_byte(data + 17, 0);
            regs[96] = 0;    // All good 
            regs[97] = 18;    // 18 bytes returned 
            break;
        case 0x28:    // Read (10) 
            sector = (read_byte(command + 4) << 8) | read_byte(command + 5);
            total = (read_byte(command + 7) << 8) | read_byte(command + 8);
            console.debug("Reading sector " + sector + ", length " + total + " (addr=0x" + data.toString(16) + ")\n");
            for (count = 0; count < total; count++) {
                pos = sector * 512;
                if (unit == (0xfffffffe >>> 0)) {
                    for (c = 0; c < 512; c++) {
                        write_byte(data, floppy1[pos + c]);
                        data++;
                    }
                } else {
                    for (c = 0; c < 512; c++) {
                        write_byte(data, floppy2[pos + c]);
                        data++;
                    }
                }
                sector++;
            }
            regs[96] = 0;
            regs[97] = total * 512;
            break;
        case 0x2a:    // Write (10) 
            sector = (read_byte(command + 4) << 8) | read_byte(command + 5);
            total = (read_byte(command + 7) << 8) | read_byte(command + 8);
            console.debug("Writing sector " + sector + ", length " + total + " (addr=0x" + data.toString(16) + ")\n");
            for (count = 0; count < total; count++) {
                pos = sector * 512;
                if (unit == (0xfffffffe >>> 0)) {
                    for (c = 0; c < 512; c++) {
                        floppy1[pos + c] = read_byte(data);
                        data++;
                    }
                } else {
                    for (c = 0; c < 512; c++) {
                        floppy2[pos + c] = read_byte(data);
                        data++;
                    }
                }
                sector++;
            }
            regs[96] = 0;
            regs[97] = total * 512;
            break;
        default:    // Unsupported commannd 
            regs[96] = 2;    // Unsupported command 
            regs[97] = 0;
            break;
    }
}

function am29000() {
    var c;
    var d;
    var instruction;
    
    for (c = 0; c < rom.length; c++) {
        rom[c] = 0xffffffff >> 0;
    }
    for (c = 0; c < rom.length; c++) {
        memory[c] = 0xffffffff >> 0;
    }
    
    // Load boot code
    d = 0xbfff0000 >>> 0;
    for (c = 0; c < 9216; c += 4) {
        instruction = ((floppy1[c] << 24) | (floppy1[c + 1] << 16) | (floppy1[c + 2] << 8) | floppy1[c + 3]) >>> 0;
        write_word(d, instruction);
        d = (d + 4) >>> 0;
    }
    pc1 = 0xbfff0000 >>> 0;
    pc0 = (pc1 + 4) >>> 0;
    endianness = 3;
    for (c = 0; c < 256; c++)
        special[c] = 0;
    for (c = 0; c < 256; c++)
        regs[c] = 0;
    write_word(0xbfffffd8, 0x00000200); // 512 kilobytes of RAM
    write_word(0xbfffffdc, 0x40); // Boot drive (0x40 for A, 0-7 for C-I)
    write_word(0xbfffffe0, 12); // 12 mhz ???
    // Patch RAM with boot loader services
    write_word(0xbfffffec, 0x00002000);
    
    // Text drawing routine, gr64=x, gr65=y, gr66=text
    rom[0x00002018 / 4] = 0xc0000080;   // JMPI lr0
    rom[0x0000201c / 4] = 0x70406161;   // NOP
    // Read disk, gr111=track and head, gr113=target address, gr77=bytes
    rom[0x00002020 / 4] = 0xfc000180 >>> 0;   // Use a non-implemented instruction
    rom[0x00002024 / 4] = 0x70406161;   // NOP
    // Box drawing routine, gr64=x, gr65=y, gr66=w, gr67=h, gr68=title
    rom[0x00002050 / 4] = 0xc0000080;   // JMPI lr0
    rom[0x00002054 / 4] = 0x70406161;   // NOP
    // Icon drawing routine, gr64=x, gr65=y, gr66=icon
    rom[0x00002058 / 4] = 0xc0000080 >>> 0;   // JMPI lr0
    rom[0x0000205c / 4] = 0x70406161;   // NOP
    // Text drawing routine, gr64=x, gr65=y, gr66=text
    rom[0x00002078 / 4] = 0xc0000080;   // JMPI lr0
    rom[0x0000207c / 4] = 0x70406161;   // NOP

}

am29000.prototype.AM29K_TRUE = 0x80000000 >>> 0;
am29000.prototype.AM29K_FALSE = 0;

// Count leading zeroes.
am29000.prototype.clz = function (value)
{
    var c;
    
    c = 0;
    if ((value >>> 24) == 0) {
        c += 8;
        value <<= 8;
        if ((value >>> 24) == 0) {
            c += 8;
            value <<= 8;
            if ((value >>> 24) == 0) {
                c += 8;
                value <<= 8;
            }
        }
    }
    return c + lz[value >>> 24];
};

// Handle trap.
am29000.prototype.trap = function (number)
{
//    console.debug("Handling trap 0x" + number.toString(16) + "\n");
    special[1] = special[2];    // Page 3-55 s
    special[2] = (special[2] & 0xc10c) | 0x0473;    // Figure 3-34
    special[10] = pc0;
    special[11] = pc1;
    special[12] = pc2;
    if ((special[3] & 0x10) == 0) {
        pc1 = ((special[0] & 0xffff0000) | (number << 8)) >>> 0;
        pc0 = (pc1 + 4) >>> 0;
    } else {
        throw "Vector Fetch table not implemented\n";
    }
};

// Special registers of the AMD Am29k processors.
//
// The original Am29000 manual doesn't have official abbreviated names
// for the special registers. These names come from the Am29050
// manual.
//
var special_regs = [
    "VAB", "OPS", "CPS", "CFG", "CHA", "CHD", "CHC", "RBP",
    "TMC", "TMR", "PC0", "PC1", "PC2", "MMU", "LRU", "RSN",
    "RMA0", "RMC0", "RMA1", "RMC1", "SPC0", "SPC1", "SPC2", "IBA0",
    "IBC0", "IBA1", "IBC1", "?", "?", "?", "?", "?",
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
    "IPC", "IPA", "IPB", "Q", "ALU", "BP", "FC", "CR",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "FPE", "INTE", "FPS", "?", "EXOP", "?", "?", "?",
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
    "?", "?", "?", "?", "?", "?", "?", "?"];

var mnemonic = [
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
    "???", "FDMUL", "???", "???", "nanochess_emulator %v,%b", "???", "???", "???"];

// Disassemble an instruction
function disassemble(pc, instruction)
{
    var q;
    var reg;
    var addr;
    var c;
    var p;
    
    p = "";
    
    /*
     ** The Am29000 doesn't have an instruction named NOP, but
     ** this is suggested by the AMD manuals.
     */
    if ((instruction >>> 24) == 0x70) {  /* ASEQ */
        if (((instruction >> 8) & 0xff) == (instruction & 0xff)) {  /* Both registers are the same */
            return "NOP";
        }
    }
    q = mnemonic[instruction >>> 24];    /* Get decoding information */
    c = 0;
    while (c < q.length) {
        if (q[c] == '%') {
            c++;
            switch (q[c++]) {
                case 'c':   /* Reg. C */
                    reg = (instruction >> 16) & 0xff;
                    p += (reg < 128 ? "gr" : "lr") + (reg & 127);
                    break;
                case 'a':   /* Reg. A */
                    reg = (instruction >> 8) & 0xff;
                    p += (reg < 128 ? "gr" : "lr") + (reg & 127);
                    break;
                case 'b':   /* Reg. B */
                    reg = (instruction & 0xff);
                    p += (reg < 128 ? "gr" : "lr") + (reg & 127);
                    break;
                case 'l':   /* LOAD/STORE memory control */
                    p += ((instruction >> 23) & 1) + ",0x" + ((instruction >> 16) & 0x7f).toString(16);
                    break;
                case 'i':   /* 8-bit immediate */
                    p += "0x" + (instruction & 0xff).toString(16);
                    break;
                case 's':   /* Special register */
                    p += special_regs[(instruction >> 8) & 0xff];
                    break;
                case 'v':   /* Instruction code (emulator internal) */
                    p += "0x" + ((instruction >> 8) & 0xff).toString(16);
                    break;
                case 't':   /* Trap 8-bit immediate */
                    p += "0x" + ((instruction >> 16) & 0xff).toString(16);
                    break;
                case 'm':   /* 16-bit immediate */
                    p += "0x" + ((instruction & 0xff) | ((instruction >> 8) & 0xff00)).toString(16);
                    break;
                case 'r':   /* Relative address */
                    addr = ((instruction & 0xff) | ((instruction >> 8) & 0xff00)) << 2;
                    if (addr & 0x020000)
                        addr -= 0x040000;
                    addr = (addr + pc) >>> 0;
                    p += "0x" + addr.toString(16);
                    break;
                case 'u':   /* Absolute address */
                    addr = ((instruction & 0xff) | ((instruction >> 8) & 0xff00)) << 2;
                    p += "0x" + addr.toString(16);
                    break;
                case 'f':   /* CONVERT */
                    p += ((instruction >> 7) & 1) + "," + ((instruction >> 4) & 7) + "," + ((instruction >> 2) & 3) + "," + (instruction & 3);
                    break;
            }
        } else {
            p += q[c++];
        }
    }
    return p;
}

am29000.prototype.get_rc = function (instruction) {
    var reg;
    
    reg = (instruction >> 16) & 0xff;
    if (reg == 0)   // Indirect
        return (special[128] >> 2) & 0xff;
    if (reg < 128)  // Global register
        return reg;
    return ((reg + (regs[1] >>> 2)) | 0x80) & 0xff; // Local register
};

am29000.prototype.get_ra = function (instruction) {
    var reg;
    
    reg = (instruction >> 8) & 0xff;
    if (reg == 0)   // Indirect
        return (special[129] >> 2) & 0xff;
    if (reg < 128)  // Global register
        return reg;
    return ((reg + (regs[1] >>> 2)) | 0x80) & 0xff; // Local register
};

am29000.prototype.get_rb = function (instruction) {
    var reg;
    
    reg = instruction & 0xff;
    if (reg == 0)   // Indirect
        return (special[130] >> 2) & 0xff;
    if (reg < 128)  // Global register
        return reg;
    return ((reg + (regs[1] >>> 2)) | 0x80) & 0xff; // Local register
};

am29000.prototype.imm16 = function (instruction) {
    return ((instruction >> 8) & 0xff00) | (instruction & 0xff);
};

am29000.prototype.imm16s = function (instruction) {
    var c;
    
    c = ((instruction >> 8) & 0xff00) | (instruction & 0xff);
    if (c >= 0x8000)
        return (c - 0x10000) >>> 0;
    return c >>> 0;
};

am29000.prototype.read_bp = function () {
    return ((special[132] >> 5) & 3);
};

am29000.prototype.write_bp = function (value) {
    special[132] = (special[132] & ~0x60) | ((value & 3) << 5);
};

am29000.prototype.read_fc = function () {
    return (special[132] & 0x1f);
};

am29000.prototype.write_fc = function (value) {
    special[132] = (special[132] & ~0x1f) | (value & 0x1f);
};

am29000.prototype.alu_carry = function () {
    return (special[132] >> 7) & 1;
};

am29000.prototype.alu = function (v1, v2, c) {
    if ((special[2] & 0x0400) == 0) {
      var tmp = v1 + v2 + c;
        
      if (tmp > (0xffffffff >>> 0))
          special[132] = special[132] | 0x80;
      else
          special[132] = special[132] & ~0x80;
      if (tmp & (0x80000000 >>> 0))
          special[132] = special[132] | 0x0200;
      else
          special[132] = special[132] & ~0x0200;
      if ((tmp & (0xffffffff >>> 0)) == 0)
          special[132] = special[132] | 0x0100;
      else
          special[132] = special[132] & ~0x0100;
      special[132] = special[132] & ~0x0400; // No overflow
    }
}

am29000.prototype.alu_simple = function (v) {
    if ((special[2] & 0x0400) == 0) {
      if (v & (0x80000000 >>> 0))
          special[132] = special[132] | 0x0200;
      else
          special[132] = special[132] & ~0x0200;
      if ((v & (0xffffffff >>> 0)) == 0)
          special[132] = special[132] | 0x0100;
      else
          special[132] = special[132] & ~0x0100;
    }
}

am29000.prototype.start_emulation = function () {
    var instruction;
    var c;
    var d;
    var e;
    var f;
    var shift;
    
    do {
        // Timer
        if ((special[8] & 0xffffff) == 0) { // Timer reload
            special[8] = special[9] & 0xffffff; // Copy value
            if (special[9] & 0x02000000)    // Interrupt already happened?
                special[9] |= 0x04000000;   // Overflow
            special[9] |= 0x02000000;   // Interrupt
        } else {
            special[8] = special[8] - 1;    // Count down
        }
        if ((special[9] & 0x03000000) == 0x03000000) {  // Interrupt + IE
            if ((special[2] & 0x0401) == 0) { // DA = 0
                special[10] = pc0;
                special[11] = pc1;
                special[12] = pc2;
                this.trap(14);
            }
        }
        
        instruction = read_word(pc1);   // Read the next instruction to execute
        
        if ((special[2] & 0x0400) == 0) {   // Update PCs only if not in Freeze mode
            special[10] = pc0;
            special[11] = pc1;
            special[12] = pc2;
        }
        count++;
        if (debug) {
            var p;
            
            p = disassemble(pc1, instruction);
            p = count + " PC=0x" + pc1.toString(16) + " " + p;
            last_instruction[current_instruction] = p;
            current_instruction = (current_instruction + 1) & 0x03ff;
        }
        pc2 = pc1;
        pc1 = pc0;
        pc0 = (pc1 + 4) >>> 0;
        switch (instruction >>> 24) {    // Decode instruction
            case 0x01:  // CONSTN
                regs[this.get_ra(instruction)] = (0xffff0000 | this.imm16(instruction)) >>> 0;
                break;
            case 0x02:  // CONSTH
                c = this.get_ra(instruction);
                regs[c] = ((regs[c] & 0xffff) | (this.imm16(instruction) << 16)) >>> 0;
                break;
            case 0x03:  // CONST
                regs[this.get_ra(instruction)] = this.imm16(instruction);
                break;
            case 0x04:  // MTSRIM
                c = (instruction >> 8) & 0xff;
                if (c == 133)
                    this.write_bp(this.imm16(instruction));
                else if (c == 134)
                    this.write_fc(this.imm16(instruction));
                else {
                    special[c] = this.imm16(instruction);
                    if (c == 3) {
                        if ((special[c] & 4) == 0) {    // BO = 0
                            endianness = 3; // Big-endian
                        } else {    // BO = 1
                            endianness = 0; // Little-endian
                        }
                    }
                }
                // !!! Add masks
                break;
            case 0x05:  // CONSTHZ
                regs[this.get_ra(instruction)] = (this.imm16(instruction) << 16) >>> 0;
                break;
            case 0x08:  // CLZ
/*                if (0) {
                    last_instruction[(current_instruction - 1) & 0x03ff] += " " + this.clz(regs[this.get_rb(instruction)]) + "=" + hex(regs[this.get_rb(instruction)]);
                    current_instruction = (current_instruction + 1) & 0x03ff;
                }*/
                regs[this.get_rc(instruction)] = this.clz(regs[this.get_rb(instruction)]);
                break;
            case 0x09:  // CLZ imm
                regs[this.get_rc(instruction)] = this.clz(instruction & 0xff);
                break;
            case 0x0a:  // EXBYTE
                c = regs[this.get_rb(instruction)] & ~0xff;
                d = this.read_bp() ^ endianness;
                if (d == 3) {
                    c |= (regs[this.get_ra(instruction)] >> 24) & 0xff;
                } else if (d == 2) {
                    c |= (regs[this.get_ra(instruction)] >> 16) & 0xff;
                } else if (d == 1) {
                    c |= (regs[this.get_ra(instruction)] >> 8) & 0xff;
                } else {
                    c |= regs[this.get_ra(instruction)] & 0xff;
                }
                regs[this.get_rc(instruction)] = c >>> 0;
                break;
            case 0x0b:  // EXBYTE imm
                c = (instruction & 0xff) & ~0xff;
                d = this.read_bp() ^ endianness;
                if (d == 3) {
                    c |= (regs[this.get_ra(instruction)] >> 24) & 0xff;
                } else if (d == 2) {
                    c |= (regs[this.get_ra(instruction)] >> 16) & 0xff;
                } else if (d == 1) {
                    c |= (regs[this.get_ra(instruction)] >> 8) & 0xff;
                } else {
                    c |= regs[this.get_ra(instruction)] & 0xff;
                }
                regs[this.get_rc(instruction)] = c >>> 0;
                break;
            case 0x0c:  // INBYTE
                c = regs[this.get_rb(instruction)] & 0xff;
                d = this.read_bp() ^ endianness;
                if (d == 3) {
                    c = (regs[this.get_ra(instruction)] & ~0xff000000) | (c << 24);
                } else if (d == 2) {
                    c = (regs[this.get_ra(instruction)] & ~0x00ff0000) | (c << 16);
                } else if (d == 1) {
                    c = (regs[this.get_ra(instruction)] & ~0x0000ff00) | (c << 8);
                } else {
                    c = (regs[this.get_ra(instruction)] & ~0x000000ff) | c;
                }
                regs[this.get_rc(instruction)] = c >>> 0;
                break;
            case 0x0d:  // INBYTE imm
                c = (instruction & 0xff);
                d = this.read_bp() ^ endianness;
                if (d == 3) {
                    c = (regs[this.get_ra(instruction)] & ~0xff000000) | (c << 24);
                } else if (d == 2) {
                    c = (regs[this.get_ra(instruction)] & ~0x00ff0000) | (c << 16);
                } else if (d == 1) {
                    c = (regs[this.get_ra(instruction)] & ~0x0000ff00) | (c << 8);
                } else {
                    c = (regs[this.get_ra(instruction)] & ~0x000000ff) | c;
                }
                regs[this.get_rc(instruction)] = c >>> 0;
                break;
            case 0x10:    // ADDS
                this.alu(regs[this.get_ra(instruction)], regs[this.get_rb(instruction)], 0);
                regs[this.get_rc(instruction)] = (regs[this.get_ra(instruction)] + regs[this.get_rb(instruction)]) >>> 0;
                // !!! Should cause a trap in case of overflow
                break;
            case 0x11:    // ADDS imm
                this.alu(regs[this.get_ra(instruction)], (instruction & 0xff), 0);
                regs[this.get_rc(instruction)] = (regs[this.get_ra(instruction)] + (instruction & 0xff)) >>> 0;
                // !!! Should cause a trap in case of overflow
                break;
            case 0x12:    // ADDU
                this.alu(regs[this.get_ra(instruction)], regs[this.get_rb(instruction)], 0);
                regs[this.get_rc(instruction)] = (regs[this.get_ra(instruction)] + regs[this.get_rb(instruction)]) >>> 0;
                // !!! Should cause a trap in case of overflow
                break;
            case 0x13:    // ADDU imm
                this.alu(regs[this.get_ra(instruction)], (instruction & 0xff), 0);
                regs[this.get_rc(instruction)] = (regs[this.get_ra(instruction)] + (instruction & 0xff)) >>> 0;
                // !!! Should cause a trap in case of overflow
                break;
            case 0x14:    // ADD
                this.alu(regs[this.get_ra(instruction)], regs[this.get_rb(instruction)], 0);
                regs[this.get_rc(instruction)] = (regs[this.get_ra(instruction)] + regs[this.get_rb(instruction)]) >>> 0;
                break;
            case 0x15:    // ADD imm
                this.alu(regs[this.get_ra(instruction)], (instruction & 0xff), 0);
                regs[this.get_rc(instruction)] = (regs[this.get_ra(instruction)] + (instruction & 0xff)) >>> 0;
                break;
            case 0x16:  // LOAD
                switch ((instruction >> 16) & 0xff) {
                    case 0x02:
                        regs[this.get_ra(instruction)] = clgd5429.mem_read_word(regs[this.get_rb(instruction)] / 4) >>> 0;
                        break;
                    case 0x04:
                        regs[this.get_ra(instruction)] = read_word(regs[this.get_rb(instruction)]) >>> 0;
                        break;
                    case 0x14:
                        c = regs[this.get_rb(instruction)] & 3;
                        this.write_bp(c);
                        regs[this.get_ra(instruction)] = read_word(regs[this.get_rb(instruction)]) >>> 0;
                        break;
                    case 0x41:
                        regs[this.get_ra(instruction)] = read_isa(regs[this.get_rb(instruction)]) >>> 0;
                        break;
                    default:
                        throw "Unhandled memory control 0x" + instruction.toString(16) + "\n";
                }
                break;
            case 0x17:  // LOAD imm
                switch ((instruction >> 16) & 0xff) {
                    case 0x04:
                        regs[this.get_ra(instruction)] = read_word((instruction & 0xff)) >>> 0;
                        break;
                    case 0x14:
                        c = regs[this.get_rb(instruction)] & 3;
                        this.write_bp(c);
                        regs[this.get_ra(instruction)] = read_word((instruction & 0xff)) >>> 0;
                        break;
                    case 0x41:
                        regs[this.get_ra(instruction)] = read_isa((instruction & 0xff)) >>> 0;
                        break;
                    default:
                        throw "Unhandled memory control 0x" + instruction.toString(16) + "\n";
                }
                break;
            case 0x18:    // ADDCS
                c = this.alu_carry();
                this.alu(regs[this.get_ra(instruction)], regs[this.get_rb(instruction)], c);
                regs[this.get_rc(instruction)] = (regs[this.get_ra(instruction)] + regs[this.get_rb(instruction)] + c) >>> 0;
                // !!! Should cause a trap in case of overflow
                break;
            case 0x19:    // ADDCS imm
                c = this.alu_carry();
                this.alu(regs[this.get_ra(instruction)], (instruction & 0xff), c);
                regs[this.get_rc(instruction)] = (regs[this.get_ra(instruction)] + (instruction & 0xff) + c) >>> 0;
                // !!! Should cause a trap in case of overflow
                break;
            case 0x1a:    // ADDCU
                c = this.alu_carry();
                this.alu(regs[this.get_ra(instruction)], regs[this.get_rb(instruction)], c);
                regs[this.get_rc(instruction)] = (regs[this.get_ra(instruction)] + regs[this.get_rb(instruction)] + c) >>> 0;
                // !!! Should cause a trap in case of overflow
                break;
            case 0x1b:    // ADDCU imm
                c = this.alu_carry();
                this.alu(regs[this.get_ra(instruction)], (instruction & 0xff), c);
                regs[this.get_rc(instruction)] = (regs[this.get_ra(instruction)] + (instruction & 0xff) + c) >>> 0;
                // !!! Should cause a trap in case of overflow
                break;
            case 0x1c:    // ADDC
                c = this.alu_carry();
                this.alu(regs[this.get_ra(instruction)], regs[this.get_rb(instruction)], c);
                regs[this.get_rc(instruction)] = (regs[this.get_ra(instruction)] + regs[this.get_rb(instruction)] + c) >>> 0;
                break;
            case 0x1d:    // ADDC imm
                c = this.alu_carry();
                this.alu(regs[this.get_ra(instruction)], (instruction & 0xff), c);
                regs[this.get_rc(instruction)] = (regs[this.get_ra(instruction)] + (instruction & 0xff) + c) >>> 0;
                break;
            case 0x1e:  // STORE
                switch ((instruction >> 16) & 0xff) {
                    case 0x00:
                        c = regs[this.get_rb(instruction)];
                        d = regs[this.get_ra(instruction)];
                        e = clgd5429.mem_write_byte(c / 4, d & 0xff);
                        if (e == -1)
                            throw "Unhandled memory control 0x" + instruction.toString(16) + "\n";
                        break;
                    case 0x02:
                        c = regs[this.get_rb(instruction)];
                        d = regs[this.get_ra(instruction)];
                        e = clgd5429.mem_write_word(c / 4, d & 0xffff);
                        if (e == -1)
                            throw "Unhandled memory control 0x" + instruction.toString(16) + "\n";
                        break;
                    case 0x04:
                        write_word(regs[this.get_rb(instruction)], regs[this.get_ra(instruction)]);
                        break;
                    case 0x41:
                        write_isa(regs[this.get_rb(instruction)], regs[this.get_ra(instruction)]);
                        break;
                    case 0x42:
                        write_isaw(regs[this.get_rb(instruction)], regs[this.get_ra(instruction)]);
                        break;
                    default:
                        throw "Unhandled memory control 0x" + instruction.toString(16) + "\n";
                }
                break;
            case 0x1f:  // STORE
                switch ((instruction >> 16) & 0xff) {
                    case 0x04:
                        write_word((instruction & 0xff), regs[this.get_ra(instruction)]);
                        break;
                    case 0x41:
                        write_isa((instruction & 0xff), regs[this.get_ra(instruction)]);
                        break;
                    default:
                        throw "Unhandled memory control 0x" + instruction.toString(16) + "\n";
                }
                break;
            case 0x20:    // SUBS
                this.alu(regs[this.get_ra(instruction)], (-regs[this.get_rb(instruction)]) >>> 0, 0);
                regs[this.get_rc(instruction)] = (regs[this.get_ra(instruction)] - regs[this.get_rb(instruction)]) >>> 0;
                // !!! Generate trap if signed value overflows
                break;
            case 0x21:    // SUBS imm
                this.alu(regs[this.get_ra(instruction)], (-(instruction & 0xff)) >>> 0, 0);
                regs[this.get_rc(instruction)] = (regs[this.get_ra(instruction)] - (instruction & 0xff)) >>> 0;
                // !!! Generate trap if signed value overflows
                break;
            case 0x22:    // SUBU
                this.alu(regs[this.get_ra(instruction)], (-regs[this.get_rb(instruction)]) >>> 0, 0);
                regs[this.get_rc(instruction)] = (regs[this.get_ra(instruction)] - regs[this.get_rb(instruction)]) >>> 0;
                // !!! Generate trap if unsigned value overflows
                break;
            case 0x23:    // SUBU imm
                this.alu(regs[this.get_ra(instruction)], (-(instruction & 0xff)) >>> 0, 0);
                regs[this.get_rc(instruction)] = (regs[this.get_ra(instruction)] - (instruction & 0xff)) >>> 0;
                // !!! Generate trap if unsigned value overflows
                break;
            case 0x24:    // SUB
                this.alu(regs[this.get_ra(instruction)], (-regs[this.get_rb(instruction)]) >>> 0, 0);
                regs[this.get_rc(instruction)] = (regs[this.get_ra(instruction)] - regs[this.get_rb(instruction)]) >>> 0;
                break;
            case 0x25:    // SUB imm
                this.alu(regs[this.get_ra(instruction)], (-(instruction & 0xff)) >>> 0, 0);
                regs[this.get_rc(instruction)] = (regs[this.get_ra(instruction)] - (instruction & 0xff)) >>> 0;
                break;
            case 0x28:    // SUBCS
                c = this.alu_carry();
                this.alu(regs[this.get_ra(instruction)], (~regs[this.get_rb(instruction)]) >>> 0, c);
                regs[this.get_rc(instruction)] = (regs[this.get_ra(instruction)] + ~regs[this.get_rb(instruction)] + c) >>> 0;
                // !!! Generate trap if signed value overflows
                break;
            case 0x29:    // SUBCS imm
                c = this.alu_carry();
                this.alu(regs[this.get_ra(instruction)], (~(instruction & 0xff)) >>> 0, c);
                regs[this.get_rc(instruction)] = (regs[this.get_ra(instruction)] + ~(instruction & 0xff) + c) >>> 0;
                // !!! Generate trap if signed value overflows
                break;
            case 0x2a:    // SUBCU
                c = this.alu_carry();
                this.alu(regs[this.get_ra(instruction)], (~regs[this.get_rb(instruction)]) >>> 0, c);
                regs[this.get_rc(instruction)] = (regs[this.get_ra(instruction)] + ~regs[this.get_rb(instruction)] + c) >>> 0;
                // !!! Generate trap if unsigned value overflows
                break;
            case 0x2b:    // SUBCU imm
                c = this.alu_carry();
                this.alu(regs[this.get_ra(instruction)], (~(instruction & 0xff)) >>> 0, c);
                regs[this.get_rc(instruction)] = (regs[this.get_ra(instruction)] + ~(instruction & 0xff) + c) >>> 0;
                // !!! Generate trap if unsigned value overflows
                break;
            case 0x2c:    // SUBC
                c = this.alu_carry();
                this.alu(regs[this.get_ra(instruction)], (~regs[this.get_rb(instruction)]) >>> 0, c);
                regs[this.get_rc(instruction)] = (regs[this.get_ra(instruction)] + ~regs[this.get_rb(instruction)] + c) >>> 0;
                break;
            case 0x2d:    // SUBC imm
                c = this.alu_carry();
                this.alu(regs[this.get_ra(instruction)], (~(instruction & 0xff)) >>> 0, c);
                regs[this.get_rc(instruction)] = (regs[this.get_ra(instruction)] + ~(instruction & 0xff) + c) >>> 0;
                break;
            case 0x2e:    // CPBYTE
                c = regs[this.get_ra(instruction)] ^ regs[this.get_rb(instruction)];
                if ((c & 0xff) == 0 || (c & 0xff00) == 0 || (c & 0xff0000) == 0 || (c & 0xff000000) == 0) {
                    regs[this.get_rc(instruction)] = this.AM29K_TRUE;
                } else {
                    regs[this.get_rc(instruction)] = this.AM29K_FALSE;
                }
                break;
            case 0x2f:    // CPBYTE imm
                c = regs[this.get_ra(instruction)] ^ (instruction & 0xff);
                if ((c & 0xff) == 0 || (c & 0xff00) == 0 || (c & 0xff0000) == 0 || (c & 0xff000000) == 0) {
                    regs[this.get_rc(instruction)] = this.AM29K_TRUE;
                } else {
                    regs[this.get_rc(instruction)] = this.AM29K_FALSE;
                }
                break;
            case 0x30:    // SUBRS
                this.alu((-regs[this.get_ra(instruction)]) >>> 0, regs[this.get_rb(instruction)], 0);
                regs[this.get_rc(instruction)] = (regs[this.get_rb(instruction)] - regs[this.get_ra(instruction)]) >>> 0;
                // !!! Generate trap if signed value overflows
                break;
            case 0x31:    // SUBRS imm
                this.alu((-regs[this.get_ra(instruction)]) >>> 0, (instruction & 0xff), 0);
                regs[this.get_rc(instruction)] = ((instruction & 0xff) - regs[this.get_ra(instruction)]) >>> 0;
                // !!! Generate trap if signed value overflows
                break;
            case 0x32:    // SUBRU
                this.alu((-regs[this.get_ra(instruction)]) >>> 0, regs[this.get_rb(instruction)], 0);
                // !!! Generate trap if unsigned value overflows
                regs[this.get_rc(instruction)] = (regs[this.get_rb(instruction)] - regs[this.get_ra(instruction)]) >>> 0;
                break;
            case 0x33:    // SUBRU imm
                this.alu((-regs[this.get_ra(instruction)]) >>> 0, (instruction & 0xff), 0);
                regs[this.get_rc(instruction)] = ((instruction & 0xff) - regs[this.get_ra(instruction)]) >>> 0;
                // !!! Generate trap if unsigned value overflows
                break;
            case 0x34:    // SUBR
                this.alu((-regs[this.get_ra(instruction)]) >>> 0, regs[this.get_rb(instruction)], 0);
                regs[this.get_rc(instruction)] = (regs[this.get_rb(instruction)] - regs[this.get_ra(instruction)]) >>> 0;
                break;
            case 0x35:    // SUBR imm
                this.alu((-regs[this.get_ra(instruction)]) >>> 0, (instruction & 0xff), 0);
                regs[this.get_rc(instruction)] = ((instruction & 0xff) - regs[this.get_ra(instruction)]) >>> 0;
                break;
            case 0x36:  // LOADM
                switch ((instruction >> 16) & 0xff) {
                    case 0x04:
                        c = regs[this.get_rb(instruction)];
                        d = this.get_ra(instruction);
                        while (1) {
                            regs[d] = read_word(c) >>> 0;
                            c = (c + 4) >>> 0;
                            if (special[135] == 0)
                                break;
                            special[135] = (special[135] - 1) & 0xff;
                            d = (((d + 1) & 0x7f) | (d & 0x80));
                        }
                        break;
                    default:
                        throw "Unhandled memory control 0x" + instruction.toString(16) + "\n";
                }
                break;
            case 0x38:    // SUBRCS
                c = this.alu_carry();
                this.alu((~regs[this.get_ra(instruction)]) >>> 0, regs[this.get_rb(instruction)], c);
                regs[this.get_rc(instruction)] = (~regs[this.get_ra(instruction)] + regs[this.get_rb(instruction)] + c) >>> 0;
                // !!! Generate trap if signed value overflows
                break;
            case 0x39:    // SUBRCS imm
                c = this.alu_carry();
                this.alu((~regs[this.get_ra(instruction)]) >>> 0, (instruction & 0xff), c);
                regs[this.get_rc(instruction)] = (~regs[this.get_ra(instruction)] + (instruction & 0xff) + c) >>> 0;
                // !!! Generate trap if signed value overflows
                break;
            case 0x3a:    // SUBRCU
                c = this.alu_carry();
                this.alu((~regs[this.get_ra(instruction)]) >>> 0, regs[this.get_rb(instruction)], c);
                regs[this.get_rc(instruction)] = (~regs[this.get_ra(instruction)] + regs[this.get_rb(instruction)] + c) >>> 0;
                // !!! Generate trap if unsigned value overflows
                break;
            case 0x3b:    // SUBRCU imm
                c = this.alu_carry();
                this.alu((~regs[this.get_ra(instruction)]) >>> 0, (instruction & 0xff), c);
                regs[this.get_rc(instruction)] = (~regs[this.get_ra(instruction)] + (instruction & 0xff) + c) >>> 0;
                // !!! Generate trap if unsigned value overflows
                break;
            case 0x3c:    // SUBRC
                c = this.alu_carry();
                this.alu((~regs[this.get_ra(instruction)]) >>> 0, regs[this.get_rb(instruction)], c);
                regs[this.get_rc(instruction)] = (~regs[this.get_ra(instruction)] + regs[this.get_rb(instruction)] + c) >>> 0;
                break;
            case 0x3d:    // SUBRC imm
                c = this.alu_carry();
                this.alu((~regs[this.get_ra(instruction)]) >>> 0, (instruction & 0xff), c);
                regs[this.get_rc(instruction)] = (~regs[this.get_ra(instruction)] + (instruction & 0xff) + c) >>> 0;
                break;
            case 0x3e:  // STOREM
                switch ((instruction >> 16) & 0xff) {
                    case 0x04:
                        c = regs[this.get_rb(instruction)];
                        d = this.get_ra(instruction);
                        while (1) {
                            write_word(c, regs[d]);
                            c = (c + 4) >>> 0;
                            if (special[135] == 0)
                                break;
                            special[135] = (special[135] - 1) & 0xff;
                            d = (((d + 1) & 0x7f) | (d & 0x80));
                        }
                        break;
                    default:
                        throw "Unhandled memory control 0x" + instruction.toString(16) + "\n";
                }
                break;
            case 0x40:    // CPLT
                e = regs[this.get_ra(instruction)] >> 0;
                f = regs[this.get_rb(instruction)] >> 0;
                regs[this.get_rc(instruction)] = (e < f) ? this.AM29K_TRUE : this.AM29K_FALSE;
                break;
            case 0x41:    // CPLT imm
                e = regs[this.get_ra(instruction)] >> 0;
                f = (instruction & 0xff) >> 0;
                regs[this.get_rc(instruction)] = (e < f) ? this.AM29K_TRUE : this.AM29K_FALSE;
                break;
            case 0x42:    // CPLTU
                c = regs[this.get_ra(instruction)];
                d = regs[this.get_rb(instruction)];
                regs[this.get_rc(instruction)] = (c < d) ? this.AM29K_TRUE : this.AM29K_FALSE;
                break;
            case 0x43:    // CPLTU imm
                c = regs[this.get_ra(instruction)];
                d = (instruction & 0xff);
                regs[this.get_rc(instruction)] = (c < d) ? this.AM29K_TRUE : this.AM29K_FALSE;
                break;
            case 0x44:    // CPLE
                e = regs[this.get_ra(instruction)] >> 0;
                f = regs[this.get_rb(instruction)] >> 0;
                regs[this.get_rc(instruction)] = (e <= f) ? this.AM29K_TRUE : this.AM29K_FALSE;
                break;
            case 0x45:    // CPLE imm
                e = regs[this.get_ra(instruction)] >> 0;
                f = (instruction & 0xff) >> 0;
                regs[this.get_rc(instruction)] = (e <= f) ? this.AM29K_TRUE : this.AM29K_FALSE;
                break;
            case 0x46:    // CPLEU
                c = regs[this.get_ra(instruction)];
                d = regs[this.get_rb(instruction)];
                regs[this.get_rc(instruction)] = (c <= d) ? this.AM29K_TRUE : this.AM29K_FALSE;
                break;
            case 0x47:    // CPLEU imm
                c = regs[this.get_ra(instruction)];
                d = (instruction & 0xff);
                regs[this.get_rc(instruction)] = (c <= d) ? this.AM29K_TRUE : this.AM29K_FALSE;
                break;
            case 0x48:    // CPGT
                e = regs[this.get_ra(instruction)] >> 0;
                f = regs[this.get_rb(instruction)] >> 0;
                regs[this.get_rc(instruction)] = (e > f) ? this.AM29K_TRUE : this.AM29K_FALSE;
                break;
            case 0x49:    // CPGT imm
                e = regs[this.get_ra(instruction)] >> 0;
                f = (instruction & 0xff) >> 0;
                regs[this.get_rc(instruction)] = (e > f) ? this.AM29K_TRUE : this.AM29K_FALSE;
                break;
            case 0x4a:    // CPGTU
                c = regs[this.get_ra(instruction)];
                d = regs[this.get_rb(instruction)];
                regs[this.get_rc(instruction)] = (c > d) ? this.AM29K_TRUE : this.AM29K_FALSE;
                break;
            case 0x4b:    // CPGTU imm
                c = regs[this.get_ra(instruction)];
                d = (instruction & 0xff);
                regs[this.get_rc(instruction)] = (c > d) ? this.AM29K_TRUE : this.AM29K_FALSE;
                break;
            case 0x4c:    // CPGE
                e = regs[this.get_ra(instruction)] >> 0;
                f = regs[this.get_rb(instruction)] >> 0;
                regs[this.get_rc(instruction)] = (e >= f) ? this.AM29K_TRUE : this.AM29K_FALSE;
                break;
            case 0x4d:    // CPGE imm
                e = regs[this.get_ra(instruction)] >> 0;
                f = (instruction & 0xff) >> 0;
                regs[this.get_rc(instruction)] = (e >= f) ? this.AM29K_TRUE : this.AM29K_FALSE;
                break;
            case 0x4e:    // CPGEU
                c = regs[this.get_ra(instruction)];
                d = regs[this.get_rb(instruction)];
                regs[this.get_rc(instruction)] = (c >= d) ? this.AM29K_TRUE : this.AM29K_FALSE;
                break;
            case 0x4f:    // CPGEU imm
                c = regs[this.get_ra(instruction)];
                d = (instruction & 0xff);
                regs[this.get_rc(instruction)] = (c >= d) ? this.AM29K_TRUE : this.AM29K_FALSE;
                break;
            case 0x50:    // ASLT
                e = regs[this.get_ra(instruction)] >> 0;
                f = regs[this.get_rb(instruction)] >> 0;
                if (e < f)
                    ;
                else
                    this.trap((instruction >> 16) & 0xff);
                break;
            case 0x51:    // ASLT imm
                e = regs[this.get_ra(instruction)] >> 0;
                f = (instruction & 0xff) >> 0;
                if (e < f)
                    ;
                else
                    this.trap((instruction >> 16) & 0xff);
                break;
            case 0x52:    // ASLTU
                c = regs[this.get_ra(instruction)];
                d = regs[this.get_rb(instruction)];
                if (c < d)
                    ;
                else
                    this.trap((instruction >> 16) & 0xff);
                break;
            case 0x53:    // ASLTU imm
                c = regs[this.get_ra(instruction)];
                d = (instruction & 0xff);
                if (c < d)
                    ;
                else
                    this.trap((instruction >> 16) & 0xff);
                break;
            case 0x54:    // ASLE
                e = regs[this.get_ra(instruction)] >> 0;
                f = regs[this.get_rb(instruction)] >> 0;
                if (e <= f)
                    ;
                else
                    this.trap((instruction >> 16) & 0xff);
                break;
            case 0x55:    // ASLE imm
                e = regs[this.get_ra(instruction)] >> 0;
                f = (instruction & 0xff) >> 0;
                if (e <= f)
                    ;
                else
                    this.trap((instruction >> 16) & 0xff);
                break;
            case 0x56:    // ASLEU
                c = regs[this.get_ra(instruction)];
                d = regs[this.get_rb(instruction)];
                if (c <= d)
                    ;
                else
                    this.trap((instruction >> 16) & 0xff);
                break;
            case 0x57:    // ASLEU imm
                c = regs[this.get_ra(instruction)];
                d = (instruction & 0xff);
                if (c <= d)
                    ;
                else
                    this.trap((instruction >> 16) & 0xff);
                break;
            case 0x58:    // ASGT
                e = regs[this.get_ra(instruction)] >> 0;
                f = regs[this.get_rb(instruction)] >> 0;
                if (e > f)
                    ;
                else
                    this.trap((instruction >> 16) & 0xff);
                break;
            case 0x59:    // ASGT imm
                e = regs[this.get_ra(instruction)] >> 0;
                f = (instruction & 0xff) >> 0;
                if (e > f)
                    ;
                else
                    this.trap((instruction >> 16) & 0xff);
                break;
            case 0x5a:    // ASGTU
                c = regs[this.get_ra(instruction)];
                d = regs[this.get_rb(instruction)];
                if (c >= d)
                    ;
                else
                    this.trap((instruction >> 16) & 0xff);
                break;
            case 0x5b:    // ASGTU imm
                c = regs[this.get_ra(instruction)];
                d = (instruction & 0xff);
                if (c >= d)
                    ;
                else
                    this.trap((instruction >> 16) & 0xff);
                break;
            case 0x5c:    // ASGE
                e = regs[this.get_ra(instruction)] >> 0;
                f = regs[this.get_rb(instruction)] >> 0;
                if (e >= f)
                    ;
                else
                    this.trap((instruction >> 16) & 0xff);
                break;
            case 0x5d:    // ASGE imm
                e = regs[this.get_ra(instruction)] >> 0;
                f = (instruction & 0xff) >> 0;
                if (e >= f)
                    ;
                else
                    this.trap((instruction >> 16) & 0xff);
                break;
            case 0x5e:    // ASGEU
                c = regs[this.get_ra(instruction)];
                d = regs[this.get_rb(instruction)];
                if (c >= d)
                    ;
                else
                    this.trap((instruction >> 16) & 0xff);
                break;
            case 0x5f:    // ASGEU imm
                c = regs[this.get_ra(instruction)];
                d = (instruction & 0xff);
                if (c >= d)
                    ;
                else
                    this.trap((instruction >> 16) & 0xff);
                break;
            case 0x60:    // CPEQ
                c = regs[this.get_ra(instruction)];
                d = regs[this.get_rb(instruction)];
                regs[this.get_rc(instruction)] = (c == d) ? this.AM29K_TRUE : this.AM29K_FALSE;
                break;
            case 0x61:    // CPEQ imm
                c = regs[this.get_ra(instruction)];
                d = (instruction & 0xff);
                regs[this.get_rc(instruction)] = (c == d) ? this.AM29K_TRUE : this.AM29K_FALSE;
                break;
            case 0x62:    // CPNEQ
                c = regs[this.get_ra(instruction)];
                d = regs[this.get_rb(instruction)];
                regs[this.get_rc(instruction)] = (c != d) ? this.AM29K_TRUE : this.AM29K_FALSE;
                break;
            case 0x63:    // CPNEQ imm
                c = regs[this.get_ra(instruction)];
                d = (instruction & 0xff);
                regs[this.get_rc(instruction)] = (c != d) ? this.AM29K_TRUE : this.AM29K_FALSE;
                break;
            case 0x64:  // MUL
                c = regs[this.get_rb(instruction)];
                d = (special[131] & 1) ? regs[this.get_ra(instruction)] : 0;
                this.alu(c, d, 0);
                e = (c + d) >>> 0;
                d = e & 0x80000000;
                if (((c ^ d) & 0x80000000) == 0 && ((c ^ e) & 0x80000000) != 0) {
                    d ^= 0x80000000;
                }
                special[131] = ((special[131] >>> 1) | (e << 31)) >>> 0;
                c = (e >>> 1) | (d >>> 0);
                regs[this.get_rc(instruction)] = c >>> 0;
                break;
            case 0x65:  // MUL imm
                c = (instruction & 0xff);
                d = (special[131] & 1) ? regs[this.get_ra(instruction)] : 0;
                this.alu(c, d, 0);
                e = (c + d) >>> 0;
                d = e & 0x80000000;
                if (((c ^ d) & 0x80000000) == 0 && ((c ^ e) & 0x80000000) != 0) {
                    d ^= 0x80000000;
                }
                special[131] = ((special[131] >>> 1) | (e << 31)) >>> 0;
                c = (e >>> 1) | (d >>> 0);
                regs[this.get_rc(instruction)] = c >>> 0;
                break;
            case 0x66:  // MULL
                c = regs[this.get_rb(instruction)];
                d = (special[131] & 1) ? regs[this.get_ra(instruction)] : 0;
                this.alu(c, (0 - d) >>> 0, 0);
                e = (c - d) >>> 0;
                d = e & 0x80000000;
                if ((c ^ d) & (c ^ e) & 0x80000000) {
                    d ^= 0x80000000;
                }
                special[131] = ((special[131] >>> 1) | (e << 31)) >>> 0;
                c = (e >>> 1) | (d >>> 0);
                regs[this.get_rc(instruction)] = c >>> 0;
                break;
            case 0x67:  // MULL imm
                c = (instruction & 0xff);
                d = (special[131] & 1) ? regs[this.get_ra(instruction)] : 0;
                this.alu(c, (0 - d) >>> 0, 0);
                e = (c - d) >>> 0;
                d = e & 0x80000000;
                if ((c ^ d) & (c ^ e) & 0x80000000) {
                    d ^= 0x80000000;
                }
                special[131] = ((special[131] >>> 1) | (e << 31)) >>> 0;
                c = (e >>> 1) | (d >>> 0);
                regs[this.get_rc(instruction)] = c >>> 0;
                break;
            case 0x68:  // DIV0
                special[132] |= 0x0800; // DF
                c = regs[this.get_rb(instruction)];
                if (c & 0x80000000)
                    special[132] |= 0x0200;
                else
                    special[132] &= ~0x0200;
                c = ((c << 1) | (special[131] >>> 31)) >>> 0;
                special[131] = (special[131] << 1) >>> 0;
                regs[this.get_rc(instruction)] = c;
                break;
            case 0x69:  // DIV0 imm
                special[132] |= 0x0800; // DF
                c = (instruction & 0xff);
                if (c & 0x80000000)
                    special[132] |= 0x0200;
                else
                    special[132] &= ~0x0200;
                c = ((c << 1) | (special[131] >>> 31)) >>> 0;
                special[131] = (special[131] << 1) >>> 0;
                regs[this.get_rc(instruction)] = c;
                break;
            case 0x6a:  // DIV
                c = regs[this.get_ra(instruction)];
                d = regs[this.get_rb(instruction)];
                if (special[132] & 0x0800) {
                    if (c >= d)
                        e = 1;
                    else
                        e = 0;
                    c = (c - d) >>> 0;
                } else {
                    if (((c + d) >>> 0) < c)
                        e = 0;
                    else
                        e = 1;
                    c = (c + d) >>> 0;
                }
                d = ((special[132] >> 9) ^ e) & 1;
                special[132] = (special[132] & ~0x0a00) | (d << 11) | ((c >> 22) & 0x0200);
                c = ((c << 1) | (special[131] >>> 31)) >>> 0;
                special[131] = ((special[131] << 1) | d) >>> 0;
                regs[this.get_rc(instruction)] = c;
                break;
            case 0x6b:  // DIV imm
                c = regs[this.get_ra(instruction)];
                d = (instruction & 0xff);
                if (special[132] & 0x0800) {
                    if (c >= d)
                        e = 1;
                    else
                        e = 0;
                    c = (c - d) >>> 0;
                } else {
                    if (((c + d) >>> 0) < c)
                        e = 0;
                    else
                        e = 1;
                    c = (c + d) >>> 0;
                }
                d = ((special[132] >> 9) ^ e) & 1;
                special[132] = (special[132] & ~0x0a00) | (d << 11) | ((c >> 22) & 0x0200);
                c = ((c << 1) | (special[131] >>> 31)) >>> 0;
                special[131] = ((special[131] << 1) | d) >>> 0;
                regs[this.get_rc(instruction)] = c;
                break;
            case 0x6c:  // DIVL
                c = regs[this.get_ra(instruction)];
                d = regs[this.get_rb(instruction)];
                if (special[132] & 0x0800) {
                    if (c >= d)
                        e = 1;
                    else
                        e = 0;
                    c = (c - d) >>> 0;
                } else {
                    if (((c + d) >>> 0) < c)
                        e = 0;
                    else
                        e = 1;
                    c = (c + d) >>> 0;
                }
                d = ((special[132] >> 9) ^ e) & 1;
                special[132] = (special[132] & ~0x0a00) | (d << 11) | ((c >> 22) & 0x0200);
                special[131] = ((special[131] << 1) | d) >>> 0;
                regs[this.get_rc(instruction)] = c;
                break;
            case 0x6d:  // DIVL imm
                c = regs[this.get_ra(instruction)];
                d = (instruction & 0xff);
                if (special[132] & 0x0800) {
                    if (c >= d)
                        e = 1;
                    else
                        e = 0;
                    c = (c - d) >>> 0;
                } else {
                    if (((c + d) >>> 0) < c)
                        e = 0;
                    else
                        e = 1;
                    c = (c + d) >>> 0;
                }
                d = ((special[132] >> 9) ^ e) & 1;
                special[132] = (special[132] & ~0x0a00) | (d << 11) | ((c >> 22) & 0x0200);
                special[131] = ((special[131] << 1) | d) >>> 0;
                regs[this.get_rc(instruction)] = c;
                break;
            case 0x6e:  // DIVREM
                if (special[132] & 0x0800) {
                    c = regs[this.get_ra(instruction)];
                } else {
                    this.alu(regs[this.get_ra(instruction)], regs[this.get_rb(instruction)], 0);
                    c = (regs[this.get_ra(instruction)] + regs[this.get_rb(instruction)]) >>> 0;
                }
                regs[this.get_rc(instruction)] = c;
                break;
            case 0x6f:  // DIVREM imm
                if (special[132] & 0x0800) {
                    c = regs[this.get_ra(instruction)];
                } else {
                    this.alu(regs[this.get_ra(instruction)], (instruction & 0xff), 0);
                    c = (regs[this.get_ra(instruction)] + (instruction & 0xff)) >>> 0;
                }
                regs[this.get_rc(instruction)] = c;
                break;
            case 0x70:    // ASEQ
                c = regs[this.get_ra(instruction)];
                d = regs[this.get_rb(instruction)];
                if (c == d)
                    ;
                else
                    this.trap((instruction >> 16) & 0xff);
                break;
            case 0x71:    // ASEQ imm
                c = regs[this.get_ra(instruction)];
                d = (instruction & 0xff);
                if (c == d)
                    ;
                else
                    this.trap((instruction >> 16) & 0xff);
                break;
            case 0x72:    // ASNEQ
                c = regs[this.get_ra(instruction)];
                d = regs[this.get_rb(instruction)];
                if (c != d)
                    ;
                else
                    this.trap((instruction >> 16) & 0xff);
                break;
            case 0x73:    // ASNEQ imm
                c = regs[this.get_ra(instruction)];
                d = (instruction & 0xff);
                if (c != d)
                    ;
                else
                    this.trap((instruction >> 16) & 0xff);
                break;
            case 0x74:  // MULU
                if (special[131] & 1) {
                    this.alu(regs[this.get_rb(instruction)], regs[this.get_ra(instruction)], 0);
                    c = (regs[this.get_rb(instruction)] + regs[this.get_ra(instruction)]) >>> 0;
                } else {
                    this.alu(regs[this.get_rb(instruction)], 0, 0);
                    c = regs[this.get_rb(instruction)];
                }
                special[131] = ((special[131] >>> 1) | (c << 31)) >>> 0;
                c = ((c >>> 1) | ((special[132] & 0x80) << 24)) >>> 0;
                regs[this.get_rc(instruction)] = c;
                break;
            case 0x75:  // MULU imm
                if (special[131] & 1) {
                    this.alu((instruction & 0xff), regs[this.get_ra(instruction)], 0);
                    c = ((instruction & 0xff) + regs[this.get_ra(instruction)]) >>> 0;
                } else {
                    this.alu((instruction & 0xff), 0, 0);
                    c = (instruction & 0xff);
                }
                special[131] = ((special[131] >>> 1) | (c << 31)) >>> 0;
                c = ((c >>> 1) | ((special[132] & 0x80) << 24)) >>> 0;
                regs[this.get_rc(instruction)] = c;
                break;
            case 0x78:  // INHW
                c = regs[this.get_rb(instruction)] & 0xffff;
                d = this.read_bp() ^ endianness;
                if ((d & 2) == 0) {
                    c = ((regs[this.get_ra(instruction)] & ~0x0000ffff) >>> 0) | c;
                } else {
                    c = ((regs[this.get_ra(instruction)] & ~0xffff0000) >>> 0) | (c << 16);
                }
                regs[this.get_rc(instruction)] = c >>> 0;
                break;
            case 0x79:  // INHW imm
                c = (instruction & 0xff) & 0xffff;
                d = this.read_bp() ^ endianness;
                if ((d & 2) == 0) {
                    c = ((regs[this.get_ra(instruction)] & ~0x0000ffff) >>> 0) | c;
                } else {
                    c = ((regs[this.get_ra(instruction)] & ~0xffff0000) >>> 0) | (c << 16);
                }
                regs[this.get_rc(instruction)] = c >>> 0;
                break;
            case 0x7a:  // EXTRACT
                c = regs[this.get_ra(instruction)];
                d = regs[this.get_rb(instruction)];
                e = this.read_fc();
                c = ((c << e) >>> 0) | ((d >>> (32 - e)) >>> 0);
                regs[this.get_rc(instruction)] = c >>> 0;
                break;
            case 0x7b:  // EXTRACT imm
                c = regs[this.get_ra(instruction)];
                d = instruction & 0xff;
                e = this.read_fc();
                c = ((c << e) >>> 0) | ((d >>> (32 - e)) >>> 0);
                regs[this.get_rc(instruction)] = c >>> 0;
                break;
            case 0x7c:  // EXHW
                c = (regs[this.get_rb(instruction)] & 0xffff0000) >>> 0;
                d = this.read_bp() ^ endianness;
                if ((d & 2) == 0) {
                    d = regs[this.get_ra(instruction)];
                } else {
                    d = regs[this.get_ra(instruction)] >>> 16;
                }
                regs[this.get_rc(instruction)] = (c | (d & 0xffff)) >>> 0;
                break;
            case 0x7d:  // EXHW imm
                c = ((instruction & 0xff) & 0xffff0000) >>> 0;
                d = this.read_bp() ^ endianness;
                if ((d & 2) == 0) {
                    d = regs[this.get_ra(instruction)];
                } else {
                    d = regs[this.get_ra(instruction)] >>> 16;
                }
                regs[this.get_rc(instruction)] = (c | (d & 0xffff)) >>> 0;
                break;
            case 0x7e:  // EXHWS
                d = this.read_bp() ^ endianness;
                if ((d & 2) == 0) {
                    d = regs[this.get_ra(instruction)];
                } else {
                    d = regs[this.get_ra(instruction)] >>> 16;
                }
                d &= 0xffff;
                if (d >= 0x8000)
                    d -= 0x10000;
                regs[this.get_rc(instruction)] = d >>> 0;
                break;
            case 0x80:    // SLL
                c = regs[this.get_ra(instruction)] << (regs[this.get_rb(instruction)] & 0x1f);
                if (0) {
                    last_instruction[(current_instruction - 1) & 0x03ff] += " " + hex(c) + "=" + hex(regs[this.get_ra(instruction)]) + "<<" + hex(regs[this.get_rb(instruction)] & 0x1f);
                    current_instruction = (current_instruction + 1) & 0x03ff;
                }
                regs[this.get_rc(instruction)] = c >>> 0;
                break;
            case 0x81:    // SLL imm
                c = regs[this.get_ra(instruction)] << ((instruction & 0xff) & 0x1f);
                regs[this.get_rc(instruction)] = c >>> 0;
                break;
            case 0x82:    // SRL
                c = regs[this.get_ra(instruction)] >>> (regs[this.get_rb(instruction)] & 0x1f);
                regs[this.get_rc(instruction)] = c >>> 0;
                break;
            case 0x83:    // SRL imm
                c = regs[this.get_ra(instruction)] >>> ((instruction & 0xff) & 0x1f);
                regs[this.get_rc(instruction)] = c >>> 0;
                break;
            case 0x86:    // SRA
                e = regs[this.get_ra(instruction)] >> (regs[this.get_rb(instruction)] & 0x1f);
                regs[this.get_rc(instruction)] = e >>> 0;
                break;
            case 0x87:    // SRA imm
                e = regs[this.get_ra(instruction)] >> ((instruction & 0xff) & 0x1f);
                regs[this.get_rc(instruction)] = e >>> 0;
                break;
            case 0x88:  // IRET
                pc0 = special[10];
                pc1 = special[11];
                special[2] = special[1];
                break;
            case 0x89:  // HALT
                throw "HALT detected.\n";
            case 0x8c:  // IRETINV
                pc0 = special[10];
                pc1 = special[11];
                special[2] = special[1];
                break;
            case 0x90:    // AND
                c = (regs[this.get_ra(instruction)] & regs[this.get_rb(instruction)]) >>> 0;
                regs[this.get_rc(instruction)] = c;
                this.alu_simple(c);
                break;
            case 0x91:    // AND imm
                c = (regs[this.get_ra(instruction)] & (instruction & 0xff)) >>> 0;
                regs[this.get_rc(instruction)] = c;
                this.alu_simple(c);
                break;
            case 0x92:    // OR
                c = (regs[this.get_ra(instruction)] | regs[this.get_rb(instruction)]) >>> 0;
                regs[this.get_rc(instruction)] = c;
                this.alu_simple(c);
                break;
            case 0x93:    // OR imm
                c = (regs[this.get_ra(instruction)] | (instruction & 0xff)) >>> 0;
                regs[this.get_rc(instruction)] = c;
                this.alu_simple(c);
                break;
            case 0x94:    // XOR
                c = (regs[this.get_ra(instruction)] ^ regs[this.get_rb(instruction)]) >>> 0;
                regs[this.get_rc(instruction)] = c;
                this.alu_simple(c);
                break;
            case 0x95:    // XOR imm
                c = (regs[this.get_ra(instruction)] ^ (instruction & 0xff)) >>> 0;
                regs[this.get_rc(instruction)] = c;
                this.alu_simple(c);
                break;
            case 0x96:    // XNOR
                c = (~(regs[this.get_ra(instruction)] ^ regs[this.get_rb(instruction)])) >>> 0;
                regs[this.get_rc(instruction)] = c;
                this.alu_simple(c);
                break;
            case 0x97:    // XNOR imm
                c = (~(regs[this.get_ra(instruction)] ^ (instruction & 0xff))) >>> 0;
                regs[this.get_rc(instruction)] = c;
                this.alu_simple(c);
                break;
            case 0x98:    // NOR
                c = (~(regs[this.get_ra(instruction)] | regs[this.get_rb(instruction)])) >>> 0;
                regs[this.get_rc(instruction)] = c;
                this.alu_simple(c);
                break;
            case 0x99:    // NOR imm
                c = (~(regs[this.get_ra(instruction)] | (instruction & 0xff))) >>> 0;
                regs[this.get_rc(instruction)] = c;
                this.alu_simple(c);
                break;
            case 0x9a:    // NAND
                c = (~(regs[this.get_ra(instruction)] & regs[this.get_rb(instruction)])) >>> 0;
                regs[this.get_rc(instruction)] = c;
                this.alu_simple(c);
                break;
            case 0x9b:    // NAND imm
                c = (~(regs[this.get_ra(instruction)] & (instruction & 0xff))) >>> 0;
                regs[this.get_rc(instruction)] = c;
                this.alu_simple(c);
                break;
            case 0x9c:    // ANDN
                c = (regs[this.get_ra(instruction)] & ~regs[this.get_rb(instruction)]) >>> 0;
                regs[this.get_rc(instruction)] = c;
                this.alu_simple(c);
                break;
            case 0x9d:    // ANDN imm
                c = (regs[this.get_ra(instruction)] & ~(instruction & 0xff)) >>> 0;
                regs[this.get_rc(instruction)] = c;
                this.alu_simple(c);
                break;
            case 0x9e:  // SETIP
                special[128] = this.get_rc(instruction) * 4;
                special[129] = this.get_ra(instruction) * 4;
                special[130] = this.get_rb(instruction) * 4;
                break;
            case 0xa0:  // JMP rel
                pc0 = (pc2 + (this.imm16s(instruction) << 2)) >>> 0;
                break;
            case 0xa1:  // JMP abs
                pc0 = (this.imm16(instruction) << 2) >>> 0;
                break;
            case 0xa4:  // JMPF rel
                if ((regs[this.get_ra(instruction)] & 0x80000000) == 0) {
                    pc0 = (pc2 + (this.imm16s(instruction) << 2)) >>> 0;
                }
                break;
            case 0xa5:  // JMPF abs
                if ((regs[this.get_ra(instruction)] & 0x80000000) == 0) {
                    pc0 = (this.imm16(instruction) << 2) >>> 0;
                }
                break;
            case 0xa8:  // CALL rel
                pc0 = (pc2 + (this.imm16s(instruction) << 2)) >>> 0;
                regs[this.get_ra(instruction)] = (pc2 + 8) >>> 0;
                break;
            case 0xa9:  // CALL abs
                pc0 = (this.imm16(instruction) << 2) >>> 0;
                regs[this.get_ra(instruction)] = (pc2 + 8) >>> 0;
                break;
            case 0xaa:    // ORN
                c = (regs[this.get_ra(instruction)] | ~regs[this.get_rb(instruction)]) >>> 0;
                regs[this.get_rc(instruction)] = c;
                this.alu_simple(c);
                break;
            case 0xab:    // ORN imm
                c = (regs[this.get_ra(instruction)] | ~(instruction & 0xff)) >>> 0;
                regs[this.get_rc(instruction)] = c;
                this.alu_simple(c);
                break;
            case 0xac:  // JMPT rel
                if ((regs[this.get_ra(instruction)] & 0x80000000) != 0) {
                    pc0 = (pc2 + (this.imm16s(instruction) << 2)) >>> 0;
                }
                break;
            case 0xad:  // JMPT abs
                if ((regs[this.get_ra(instruction)] & 0x80000000) != 0) {
                    pc0 = (this.imm16(instruction) << 2) >>> 0;
                }
                break;
            case 0xb4:  // JMPFDEC rel
                c = regs[this.get_ra(instruction)];
                regs[this.get_ra(instruction)] = (c - 1) >>> 0;
                if ((c & 0x80000000) == 0) {
                    pc0 = (pc2 + (this.imm16s(instruction) << 2)) >>> 0;
                }
                break;
            case 0xb5:  // JMPFDEC abs
                c = regs[this.get_ra(instruction)];
                regs[this.get_ra(instruction)] = (c - 1) >>> 0;
                if ((c & 0x80000000) == 0) {
                    pc0 = (this.imm16(instruction) << 2) >>> 0;
                }
                break;
            case 0xc0:  // JMPI
                pc0 = regs[this.get_rb(instruction)];
                break;
            case 0xc4:  // JMPFI
                if ((regs[this.get_ra(instruction)] & 0x80000000) == 0) {
                    pc0 = regs[this.get_rb(instruction)];
                }
                break;
            case 0xc6:  // MFSR
                c = (instruction >> 8) & 0xff;
                if (c == 133)   // BP
                    c = this.read_bp();
                else if (c == 134)   // FC
                    c = this.read_fc();
                else
                    c = special[c];
                regs[this.get_rc(instruction)] = c;
                break;
            case 0xc8:  // CALLI
                pc0 = regs[this.get_rb(instruction)];
                regs[this.get_ra(instruction)] = (pc2 + 8) >>> 0;
                break;
            case 0xcc:  // JMPTI
                if ((regs[this.get_ra(instruction)] & 0x80000000) != 0) {
                    pc0 = regs[this.get_rb(instruction)];
                }
                break;
            case 0xce:  // MTSR
                c = (instruction >> 8) & 0xff;
                if (c == 133) {
                    this.write_bp(regs[this.get_rb(instruction)]);
                } else if (c == 134) {
                    this.write_fc(regs[this.get_rb(instruction)]);
                } else {
                    special[c] = regs[this.get_rb(instruction)];
                    if (c == 3) {
                        if ((special[c] & 4) == 0) {    // BO = 0
                            endianness = 3; // Big-endian
                        } else {    // BO = 1
                            endianness = 0; // Little-endian
                        }
                    }
                }
                if (c == 0) {   // VAB
                    if (first_time) {
                        first_time = 0;
                        
                        // Patch the floppy disk code
                        write_word(0x8002086c, 0x4e618260 >>> 0); // Support two drives
                        write_word(0x80020878, 0xfc000280 >>> 0); // Call the emulator
                        
                        // Patch a math emulator strange error
                        // It makes it to crash with CONVERT gr96,lr4,0,0,2,1 because fraction is non-zero
                        // Probably the Am29000 was replaced with an Am29050 and I inserted
                        // more code without testing in the old processor.
                        
                        regs[95] = 0x00040040;  // Avoid CONVERT trap + MULTIPLY trap
                        write_word(0x800097fc, 0x70406161 >>> 0); // NOP
                    }
                }
                // !!! Add masks
                break;
            case 0xd7:  // EMULATE
                special[129] = this.get_ra(instruction) * 4;
                special[130] = this.get_rb(instruction) * 4;
                this.trap((instruction >> 16) & 0xff);
                break;
            case 0xde:  // MULTM
            case 0xdf:  // MULTMU
            case 0xe0:  // MULTIPLY
            case 0xe1:  // DIVIDE
            case 0xe2:  // MULTIPLU
            case 0xe3:  // DIVIDU
            case 0xe4:  // CONVERT
            case 0xe5:  // SQRT
            case 0xe6:  // CLASS
            case 0xe8:  // MTACC
            case 0xe9:  // MFACC
            case 0xea:  // FEQ
            case 0xeb:  // DEQ
            case 0xec:  // FGT
            case 0xed:  // DGT
            case 0xee:  // FLT
            case 0xef:  // DLT
            case 0xf0:  // FADD
            case 0xf1:  // DADD
            case 0xf2:  // FSUB
            case 0xf3:  // DSUB
            case 0xf4:  // FMUL
            case 0xf5:  // DMUL
            case 0xf6:  // FDIV
            case 0xf7:  // DDIV
                special[128] = this.get_rc(instruction) * 4;
                special[129] = this.get_ra(instruction) * 4;
                special[130] = this.get_rb(instruction) * 4;
                this.trap((instruction >> 24) & 0x3f);
                break;
            case 0xfc:  // Services
                pc0 = regs[this.get_rb(instruction)];
                switch ((instruction >> 8) & 0xff) {
                    case 0x01:
                        // Read disk, gr111=track and head, gr113=target address, gr77=bytes
                        d = regs[113];
                        f = (regs[77] > 9216) ? 9216 : regs[77];
                        c = regs[111];
                        console.debug("Reading track " + ((c / 2) >> 0) + ", side " + (c % 2) + "\n");
                        c *= 9216;
                        for (e = 0; e < f; e += 4) {
                            instruction = (floppy1[c + e] << 24) | (floppy1[c + e + 1] << 16) | (floppy1[c + e + 2] << 8) | floppy1[c + e + 3];
                            write_word(d, instruction >>> 0);
                            d = (d + 4) >>> 0;
                        }
                        break;
                    case 0x02:
                        floppy_scsi(regs[this.get_rb(130)], regs[this.get_rb(131)], regs[this.get_rb(132)], regs[this.get_rb(133)], regs[this.get_rb(134)], regs[this.get_rb(135)]);
                        break;
                    default:
                        throw "Unknown service requested (PC = 0x" + pc1.toString(16) +  ")\n";
                }
                break;
            default:
                throw "Instruction 0x" + instruction.toString(16) + " not implemented (PC = 0x" + pc1.toString(16) + ")\n";
        }
    } while (count % 10000) ;
    console.debug("Cycle " + count + " completed...\n");
};



