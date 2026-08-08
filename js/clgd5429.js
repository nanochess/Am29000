//
// CL-GD549 emulation
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
// Creation date: Aug/06/2026.
//

//
// This is a simple emulation of the Cirrus Logic GD5429, only the
// required functions for a 800x600x64k display with some
// acceleration functions and harwdware cursor.
//
// Given the slowness of the ISA bus, these features gave
// amazing speed for graphical operating systems.
//
// Technical reference manual downloaded from:
// http://www.s100computers.com/My%20System%20Pages/VGA_16_Board%20(Cirrus)/GD542x%20Technical%20Reference%20Manual.pdf
//
// Hardware cursor described in page 528.
//
// Notice there is no support for the BIOS ROM as it isn't used.
//

function clgd5429(canvas) {
    var canvas_data;
    var c;
    
    this.canvas = canvas;
    this.ctx = this.canvas.getContext("2d");
    this.ctx.imageSmoothingEnabled = false;
    canvas_data = this.ctx.getImageData(0, 0, 800, 600);
    for (c = 0; c < 800 * 600 * 4; c += 4)
        canvas_data.data[c + 3] = 255;  /* Set alpha channel to solid */
    this.ctx.putImageData(canvas_data, 0, 0);
    this.feature = 0;
    this.mstatus = 0;
    this.cstatus = 0;
    this.dac_read = 0;
    this.dac_mask = 0;
    this.hidden_dac = 0;
    this.sequencer_register = 0;
    this.sr = new Uint8Array(32);
    this.crtc_register = 0;
    this.cr = new Uint8Array(32);
    this.graphics_register = 0;
    this.gr = new Uint8Array(64);
    this.attribute_switch = 0;
    this.attribute_register = 0;
    this.ar = new Uint8Array(32);
    this.palette_pointer = 0;
    this.palette = new Uint8Array(768);
    this.cursor_x = 0;
    this.cursor_y = 0;
    this.cursor = new Uint8Array(64 * 32);
    this.ram = new Uint8Array(1048576);
}

// IO port write
clgd5429.prototype.io_write_byte = function (port, byte)
{
    var source;
    var target;
    var source_stride;
    var target_stride;
    var width;
    var height;
    
    switch (port) {
        case 0x3c0: // Attribute registers
            if (this.attribute_switch == 0)
                this.attribute_register = byte;  // Address
            else
                this.ar[this.attribute_register & 0x1f] = byte;   // Data
            this.attribute_switch ^= 1;
            break;
        case 0x3c2: // Miscellaneous output register
            this.mstatus = byte;
            break;
        case 0x3c4: // Sequencer register
            this.sequencer_register = byte;
            break;
        case 0x3c5: // Sequencer data
            //
            // The hardware cursor coordinates use the top 3 bits of the
            // register number as bits 2-0 of coordinate.
            //
            if ((this.sequencer_register & 0x1f) == 0x10) {
                this.cursor_x = (byte << 3) | (this.sequencer_register >> 5);
            } else if ((this.sequencer_register & 0x1f) == 0x11) {
                this.cursor_y = (byte << 3) | (this.sequencer_register >> 5);
            }
            this.sr[this.sequencer_register & 0x1f] = byte;
            break;
        case 0x3c6: // DAC mask
            if (this.dac_read == 4) {
                this.dac_read = 0;
                this.hidden_dac = byte;
                console.debug("Hidden DAC value set to 0x" + byte.toString(16) + "\n");
                if (byte == 0xe1) {
                    console.debug("Bitmap is 5-6-5 XGA(tm)\n");
                } else {
                    console.debug("Error: DAC value unhandled\n");
                }
                break;
            }
            this.dac_mask = byte;
            break;
        case 0x3c8: // DAC register
            this.palette_pointer = byte * 3;
            break;
        case 0x3c9: // DAC R,G,B
            this.palette[this.palette_pointer++] = byte;
            if (this.palette_pointer == 768)
                this.palette_pointer = 0;
            break;
        case 0x3ce: // Graphics register
            this.graphics_register = byte;
            break;
        case 0x3cf: // Graphics data
            this.gr[this.graphics_register & 0x3f] = byte;
            if ((this.graphics_register & 0x3f) == 0x31 && byte == 0x02) {   // Start bitblt
                // lr8 lr2 = source address
                // lr9 lr3 = target address
                // lr10 lr4 = width
                // lr11 lr5 = height
                // lr13 lr7 = target stride
                // lr14 lr8 = source stride
                // lr15 lr9 = command (0x0d = copy, 0x59 = xor)
                this.gr[0x31] = 0x01;    // Busy
                // this.gr[0x30] is Mode
                width = this.gr[0x20] | (this.gr[0x21] << 8);
                height = this.gr[0x22] | (this.gr[0x23] << 8);
                target_stride = this.gr[0x24] | (this.gr[0x25] << 8);
                source_stride = this.gr[0x26] | (this.gr[0x27] << 8);
                target = this.gr[0x28] | (this.gr[0x29] << 8) | (this.gr[0x2a] << 16); // Target address
                if (this.gr[0x30] & 0x40) {  // 8x8 pattern copy
                    var d;
                    var e;
                    var f;
                    var source2;
                    var target2;
                    
                    d = 0;
                    do {
                        source = this.gr[0x2c] | (this.gr[0x2d] << 8) | (this.gr[0x2e] << 16); // Source address
                        source += d * 16;
                        source &= 0x0fffff;
                        target &= 0x0fffff;
                        if (source + width <= 0x100000 && target + width <= 0x100000) {
                            // Operations with pattern in multiples of 8
                            if (this.gr[0x32] == 0x0d) { // Copy
                                target2 = target;
                                e = width + 1;
                                do {
                                    if (e > 16)
                                        f = 16;
                                    else
                                        f = e;
                                    source2 = source;
                                    e -= f;
                                    do {
                                        this.ram[target2++] = this.ram[source2++];
                                    } while (--f) ;
                                } while (e) ;
                            } else if (this.gr[0x32] == 0x59) {  // XOR
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
                                        this.ram[target2++] ^= this.ram[source2++];
                                    } while (--f) ;
                                } while (e) ;
                            }
                        }
                        target += target_stride;
                        d = (d + 1) & 7;    // Pattern row (0-7)
                    } while (height--) ;
                } else {
                    source = this.gr[0x2c] | (this.gr[0x2d] << 8) | (this.gr[0x2e] << 16); // Source address
                    if (this.gr[0x30] & 1) { // Copy in reverse direction
                        var p1;
                        var p2;
                        var c;

                        // Source and target addresses point to the lower-right byte.
                        do {
                            source &= 0x0fffff;
                            target &= 0x0fffff;
                            if (source - width >= 0 && target - width >= 0) {
                                p1 = source;
                                p2 = target;
                                c = width + 1;
                                if (this.gr[0x32] == 0x0d) {
                                    do {
                                        this.ram[p2--] = this.ram[p1--];
                                    } while (--c) ;
                                } else if (this.gr[0x32] == 0x59) {
                                    do {
                                        this.ram[p2--] ^= this.ram[p1--];
                                    } while (--c) ;
                                }
                            }
                            source -= source_stride;
                            target -= target_stride;
                        } while (height--) ;
                    } else {
                        var p1;
                        var p2;
                        var c;
                        
                        do {
                            source &= 0x0fffff;
                            target &= 0x0fffff;
                            if (source + width <= 0x100000 && target + width <= 0x100000) {
                                p1 = source;
                                p2 = target;
                                c = width + 1;
                                if (this.gr[0x32] == 0x0d) {
                                    do {
                                        this.ram[p2++] = this.ram[p1++];
                                    } while (--c) ;
                                } else if (this.gr[0x32] == 0x59) {
                                    do {
                                        this.ram[p2++] ^= this.ram[p1++];
                                    } while (--c) ;
                                }
                            }
                            source += source_stride;
                            target += target_stride;
                        } while (height--) ;
                    }
                }
                this.gr[0x31] = 0x00;    // Not busy
            }
            break;
        case 0x3d4: // CRTC register
            this.crtc_register = byte;
            break;
        case 0x3d5: // CRTC data
            this.cr[this.crtc_register & 0x3f] = byte;
            break;
        case 0x3da: // Feature register
            this.feature = byte;
            break;
        default:
            return -1;
    }
    return 0;
};

//
// Write a 16-bit word to the I/O ports.
// It simply separates it in two bytes.
//
clgd5429.prototype.io_write_word = function (port, word)
{
    var c;
    
    c = this.io_write_byte(port, word & 0xff);
    if (c != -1)
        c = this.io_write_byte(port + 1, (word >> 8) & 0xff);
    return c;
};

//
// Read a byte from the I/O port.
//
clgd5429.prototype.io_read_byte = function (port)
{
    switch (port) {
        case 0x3c1: // Attribute registers
            this.attribute_switch ^= 1;
            return this.ar[this.attribute_register & 0x1f];
        case 0x3c2: // Miscellaneous output register
            return this.mstatus;
        case 0x3c4: // Sequencer register number
            return this.sequencer_register;
        case 0x3c5: // Sequencer register data
            return this.sr[this.sequencer_register & 0x1f];
        case 0x3c6: // DAC mask
            if (this.dac_read == 4) {
                this.dac_read = 0;
                return this.hidden_dac;
            }
            this.dac_read = this.dac_read + 1;
            return this.dac_mask;
        case 0x3ce: // Graphics register number
            return this.graphics_register;
        case 0x3cf: // Graphics register data
            return this.gr[this.graphics_register & 0x3f];
        case 0x3d4: // CRTC register number
            return this.crtc_register;
        case 0x3d5: // CRTC register data
            return this.cr[this.crtc_register & 0x3f];
        case 0x3da: // Feature register
            return this.cstatus;
        default:
            return -1;
    }
};

//
// Write a byte into RAM.
//
clgd5429.prototype.mem_write_byte = function (address, byte)
{
    var c;
    
    if (address < 0x000a0000 || address > 0x000bffff) { // A:0000 - B:FFFF
        return -1;
    }
    address -= 0x000a0000;
    address <<= 4;  // Supposes this.gr[0x0b] == 0x1c
    if (address >= 0x100000) {    // 1 MB of RAM
        console.debug("CL-GD5429: Too big address 0x" + address.toString(16) + "\n");
        return -1;
    }
    // Supposes Write Mode 4. this.gr[0x0b] & 4 opens this.gr[0x05] & 7
    if ((this.gr[0x0b] & 0x04) != 0 && (this.gr[0x05] & 7) == 4) {    // Write mode 4
        byte &= this.sr[2];  // Mask, and now expand bitmap to 16-bit pixels
        if (byte & 0x80) {
            this.ram[address + 0] = this.gr[0x01];
            this.ram[address + 1] = this.gr[0x11];
        }
        if (byte & 0x40) {
            this.ram[address + 2] = this.gr[0x01];
            this.ram[address + 3] = this.gr[0x11];
        }
        if (byte & 0x20) {
            this.ram[address + 4] = this.gr[0x01];
            this.ram[address + 5] = this.gr[0x11];
        }
        if (byte & 0x10) {
            this.ram[address + 6] = this.gr[0x01];
            this.ram[address + 7] = this.gr[0x11];
        }
        if (byte & 0x08) {
            this.ram[address + 8] = this.gr[0x01];
            this.ram[address + 9] = this.gr[0x11];
        }
        if (byte & 0x04) {
            this.ram[address + 10] = this.gr[0x01];
            this.ram[address + 11] = this.gr[0x11];
        }
        if (byte & 0x02) {
            this.ram[address + 12] = this.gr[0x01];
            this.ram[address + 13] = this.gr[0x11];
        }
        if (byte & 0x01) {
            this.ram[address + 14] = this.gr[0x01];
            this.ram[address + 15] = this.gr[0x11];
        }
    } else {
        throw "CL-GD5429: Unhandled write mode 0x" + this.gr[0x0b].toString(16) + "\n";
    }
    return 0;
};

//
// Write a 16-bit word into RAM.
//
clgd5429.prototype.mem_write_word = function (address, word)
{
    if (address < 0x000a0000 || address > 0x000bffff) {
        return -1;
    }
    address -= 0x000a0000;
    if (this.gr[0x0b] == 0x00) {
        address += (this.gr[0x09] & 0xfc) << 12;
        if (address >= 0x100000) {    // 1 MB of RAM
            console.debug("CL-GD5429: Too big address 0x" + address.toString(16) + "\n");
            return -1;
        }
        this.ram[address + 0] = word;
        this.ram[address + 1] = word >> 8;
        return 0;
    }
    throw "CL-GD5429: Unhandled write mode 0x" + this.gr[0x0b].toString(16) + "\n";
    return 0;
};

//
// Read a 16-bit word from RAM.
//
clgd5429.prototype.mem_read_word = function (address)
{
    if (address < 0x000a0000 || address > 0x000bffff) {
        return -1;
    }
    address -= 0x000a0000;
    if (this.gr[0x0b] == 0x00) {
        address += (this.gr[0x09] & 0xfc) << 12;
        if (address >= 0x100000) {    // 1 MB of RAM
            console.debugf("CL-GD5429: Too big address 0x" + address.toString(16) + "\n");
            return -1;
        }
        return (this.ram[address + 1] << 8) | this.ram[address];
    }
    throw "CL-GD5429: Unhandled read mode 0x" + this.gr[0x0b].toString(16) + "\n";
    return 0;
};

//
// Save the background under the cursor.
//
clgd5429.prototype.save_cursor = function() {
    var source;
    var y;
    var c;
    
    if ((this.sr[0x12] & 1) == 0)
        return;
    source = (this.cursor_y * 800 + this.cursor_x) * 2;
    for (y = 0; y < 32; y++) {
        source &= 0x0fffff;
        if (source + 64 > 0x100000)
            continue;
        for (c = 0; c < 64; c++)
            this.cursor[y * 64 + c] = this.ram[source + c];
        source += 800 * 2;
    }
}

//
// Restore the background under the cursor.
//
clgd5429.prototype.restore_cursor = function() {
    var source;
    var y;
    var c;

    if ((this.sr[0x12] & 1) == 0)
        return;
    source = (this.cursor_y * 800 + this.cursor_x) * 2;
    for (y = 0; y < 32; y++) {
        source &= 0x0fffff;
        if (source + 64 > 0x100000)
            continue;
        for (c = 0; c < 64; c++)
            this.ram[source + c] = this.cursor[y * 64 + c];
        source += 800 * 2;
    }
}

//
// Draw the cursor
// Currently only 32x32 pixels supported.
//
clgd5429.prototype.draw_cursor = function() {
    var source;
    var target;
    var x;
    var y;
    var c;
    var color_1;
    var color_2;
    
    if ((this.sr[0x12] & 1) == 0)    /* Hardware cursor enabled? */
        return;
    color_1 = ((this.palette[0] & 0x3e) << 10) | ((this.palette[1] & 0x3f) << 5) | ((this.palette[2] & 0x3e) >> 1);
    color_2 = ((this.palette[765] & 0x3e) << 10) | ((this.palette[766] & 0x3f) << 5) | ((this.palette[767] & 0x3e) >> 1);
    for (y = 0; y < 32; y++) {
        source = 0xfc000 + (this.sr[0x13] * 256) + y * 4;
        target = ((this.cursor_y + y) * 800 + this.cursor_x) * 2;
        if (target + 64 > 0x100000)
            continue;
        for (x = 0; x < 4; x++) {
            for (c = 0x80; c; c >>= 1) {
                if ((this.ram[source + 128] & c) == 0) {
                    if ((this.ram[source] & c) != 0) {   /* XOR pixel */
                        this.ram[target + 0] = ~this.ram[target + 0];
                        this.ram[target + 1] = ~this.ram[target + 1];
                    }
                } else {
                    if ((this.ram[source] & c) == 0) {   /* Background color */
                        this.ram[target + 0] = color_1;
                        this.ram[target + 1] = color_1 >> 8;
                    } else {
                        this.ram[target + 0] = color_2;  /* Foreground color */
                        this.ram[target + 1] = color_2 >> 8;
                    }
                }
                target += 2;
            }
            source++;
        }
    }
}

// Render video
clgd5429.prototype.render = function () {
    var canvas_data;
    var x;
    var c;
    var d;
    var e;
    var f;
    
    this.save_cursor();
    this.draw_cursor();
    canvas_data = this.ctx.getImageData(0, 0, 800, 600);
    d = 0;
    for (x = 0; x < 800 * 600 * 2; x += 2) {
        e = this.ram[x] | (this.ram[x + 1] << 8);
        canvas_data.data[d] = (e >> 8) & 0xf8;
        canvas_data.data[d + 1] = (e >> 3) & 0xfc;
        canvas_data.data[d + 2] = (e << 3) & 0xf8;
        d += 4;
    }
    this.ctx.putImageData(canvas_data, 0, 0);
    this.restore_cursor();
}
