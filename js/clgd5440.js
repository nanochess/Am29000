//
// CL-GD5440 emulation
//
// (c) Copyright 2026 Oscar Toledo G. All rights reserved.
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
// Creation date: Sep/05/2026. Based on my CL-GD5429 emulation.
//

//
// This is a simple emulation of the Cirrus Logic GD5440, only the
// required functions for a 800x600x64k display with some
// acceleration functions and harwdware cursor.
//
// Technical reference manual downloaded from:
// https://www.vgamuseum.info/index.php/cpu/item/143-cirrus-logic-cl-gd5440
//
// MMIO (Memory Mapped I/O) described in page 566.
//
// Notice there is no support for the BIOS ROM as it isn't used.
//

const CLGD5440_RAM_SIZE = 0x00200000;
const CLGD5440_RAM_MASK = 0x001fffff;

function clgd5440(canvas) {
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
    this.ram = new Uint8Array(CLGD5440_RAM_SIZE);
}

clgd5440.prototype.source = 0;
clgd5440.prototype.target = 0;
clgd5440.prototype.source_stride = 0;
clgd5440.prototype.target_stride = 0;
clgd5440.prototype.width = 0;
clgd5440.prototype.height = 0;
clgd5440.prototype.current_target = 0;
clgd5440.prototype.current_width = 0;

// Handle bitmap expansion (bitblt takes bus control)
clgd5440.prototype.bitmap_expansion = function (word)
{
    var address;
    var byte;
    var available_bytes;
    var mask;
    var c;
    
    address = this.current_target;
    available_bytes = 4;
    while (available_bytes--) {
        byte = word & 0xff;
        word >>>= 8;
        mask = 0x80;
        c = this.current_width > 16 ? 16 : this.current_width;
        this.current_width -= c;
        while (c > 0) {
            if (byte & mask) {
                this.ram[(this.current_target + 0) & CLGD5440_RAM_MASK] = this.gr[0x01];
                this.ram[(this.current_target + 1) & CLGD5440_RAM_MASK] = this.gr[0x11];
            }
            mask >>= 1;
            this.current_target += 2;
            c -= 2;
        }
        if (this.current_width == 0) {
            this.target += this.target_stride;
            this.current_target = this.target;
            this.current_width = this.width + 1;
            if (this.height == 0) {
                this.gr[0x31] = 0x00;    /* Not busy */
            } else {
                this.height--;
            }
        }
    }
}

// Start a bitblt operation
clgd5440.prototype.start_bitblt = function ()
{
    // lr8 lr2 = source address
    // lr9 lr3 = target address
    // lr10 lr4 = width
    // lr11 lr5 = height
    // lr13 lr7 = target stride
    // lr14 lr8 = source stride
    // lr15 lr9 = command (0x0d = copy, 0x59 = xor)
    this.gr[0x31] = 0x01;    // Busy
    // this.gr[0x30] is Mode
    this.width = this.gr[0x20] | (this.gr[0x21] << 8);
    this.height = this.gr[0x22] | (this.gr[0x23] << 8);
    this.target_stride = this.gr[0x24] | (this.gr[0x25] << 8);
    this.source_stride = this.gr[0x26] | (this.gr[0x27] << 8);
    this.target = this.gr[0x28] | (this.gr[0x29] << 8) | (this.gr[0x2a] << 16); // Target address
    if (this.gr[0x30] == 0x9c) { /* Bitmap expansion */
        this.current_target = this.target;
        this.current_width = this.width + 1;
        return;
    }
    if (this.gr[0x30] & 0x40) {  // 8x8 pattern copy
        var d;
        var e;
        var f;
        var source2;
        var target2;
        
        d = 0;
        do {
            this.source = this.gr[0x2c] | (this.gr[0x2d] << 8) | (this.gr[0x2e] << 16); // Source address
            this.source += d * 16;
            this.source &= CLGD5440_RAM_MASK;
            this.target &= CLGD5440_RAM_MASK;
            if (this.source + this.width < CLGD5440_RAM_SIZE && this.target + this.width < CLGD5440_RAM_SIZE) {
                // Operations with pattern in multiples of 8
                target2 = this.target;
                e = this.width + 1;
                if (this.gr[0x32] == 0x0d) { // Copy
                    do {
                        if (e > 16)
                            f = 16;
                        else
                            f = e;
                        source2 = this.source;
                        e -= f;
                        do {
                            this.ram[target2++] = this.ram[source2++];
                        } while (--f) ;
                    } while (e) ;
                } else if (this.gr[0x32] == 0x59) {  // XOR
                    do {
                        if (e > 16)
                            f = 16;
                        else
                            f = e;
                        source2 = this.source;
                        e -= f;
                        do {
                            this.ram[target2++] ^= this.ram[source2++];
                        } while (--f) ;
                    } while (e) ;
                }
            }
            this.target += this.target_stride;
            d = (d + 1) & 7;    // Pattern row (0-7)
        } while (this.height--) ;
    } else {
        this.source = this.gr[0x2c] | (this.gr[0x2d] << 8) | (this.gr[0x2e] << 16); // Source address
        if (this.gr[0x30] & 1) { // Copy in reverse direction
            var p1;
            var p2;
            var c;

            // Source and target addresses point to the lower-right byte.
            do {
                this.source &= CLGD5440_RAM_MASK;
                this.target &= CLGD5440_RAM_MASK;
                if (this.source - this.width >= 0 && this.target - this.width >= 0) {
                    p1 = this.source;
                    p2 = this.target;
                    c = this.width + 1;
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
                this.source -= this.source_stride;
                this.target -= this.target_stride;
            } while (this.height--) ;
        } else {
            var p1;
            var p2;
            var c;
            
            do {
                this.source &= CLGD5440_RAM_MASK;
                this.target &= CLGD5440_RAM_MASK;
                if (this.source + this.width <= CLGD5440_RAM_SIZE && this.target + this.width <= CLGD5440_RAM_SIZE) {
                    p1 = this.source;
                    p2 = this.target;
                    c = this.width + 1;
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
                this.source += this.source_stride;
                this.target += this.target_stride;
            } while (this.height--) ;
        }
    }
    this.gr[0x31] = 0x00;    // Not busy
}

// IO port write
clgd5440.prototype.pci_io_write_byte = function (port, byte)
{
    byte >>= 8 * (port & 3);
    byte &= 0xff;
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
                this.start_bitblt();
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
clgd5440.prototype.pci_io_write_word = function (port, word)
{
    var c;
    
    c = this.pci_io_write_byte(port, word);
    if (c != -1)
        c = this.pci_io_write_byte(port + 1, word);
    return c;
};

//
// Read a byte from the I/O port.
//
clgd5440.prototype.pci_io_read_byte = function (port)
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
clgd5440.prototype.pci_mem_write_byte = function (address, word)
{
    var c;

    if (((address & 0xffff8000) >>> 0) == 0x800b8000) { // MMIO registers
        address &= 0xfc;
        switch (address) {
            default:
                throw "CL-GD5440: Unhandled byte write address 0x" + address.toString(16) + "\n";
            case 0x40:
                this.gr[0x31] = word & 0xff;
                if ((word & 0xff) == 0x02)
                    this.start_bitblt();
                break;
        }
        return 0;
    }
    throw "CL-GD5440: Unhandled byte write address 0x" + address.toString(16) + "\n";
    return 0;
};

//
// Write a 16-bit word into RAM.
//
clgd5440.prototype.pci_mem_write_word = function (address, word)
{
    if (((address & 0xff000000) >>> 0) == 0x81000000) { // 16 mb. aperture
        if ((address & 2) == 0) {
            address &= CLGD5440_RAM_MASK & ~3;
            this.ram[address] = word;
            this.ram[address + 1] = word >> 8;
        } else {
            address &= CLGD5440_RAM_MASK & ~3;
            this.ram[address + 2] = word >> 16;
            this.ram[address + 3] = word >> 24;
        }
        return 0;
    }
    throw "CL-GD5440: Unhandled 16-bit write to 0x" + address.toString(16) + "\n";
    return 0;
};

//
// Write a 32-bit word
//
clgd5440.prototype.pci_mem_write_dword = function (address, word)
{
    if (((address & 0xc0000000) >>> 0) == 0xc0000000) {
        if ((address & 0x08000000) == 0x08000000) {
            // Ignore PCI header writes that enable the video card.
            return 0;
        }
        return 0;
    }
    if (((address & 0xff000000) >>> 0) == 0x81000000) { // 16 mb. aperture
        if (this.gr[0x31] == 0x01) {    // In this emulation subset, it must be bitmap expansion
            this.bitmap_expansion(word);
            return 0;
        }
        address &= CLGD5440_RAM_MASK & ~3;
        this.ram[address] = word;
        this.ram[address + 1] = word >> 8;
        this.ram[address + 2] = word >> 16;
        this.ram[address + 3] = word >> 24;
        return 0;
    }
    if (((address & 0xffff8000) >>> 0) == 0x800b8000) { // MMIO registers
        address &= 0xfc;
        switch (address) {
            case 0x00:
                this.gr[0x00] = word & 0xff;
                this.gr[0x10] = (word >> 8) & 0xff;
                this.gr[0x12] = (word >> 16) & 0xff;
                this.gr[0x14] = (word >> 24) & 0xff;
                break;
            case 0x04:
                this.gr[0x01] = word & 0xff;
                this.gr[0x11] = (word >> 8) & 0xff;
                this.gr[0x13] = (word >> 16) & 0xff;
                this.gr[0x15] = (word >> 24) & 0xff;
                break;
            case 0x08:
                this.gr[0x20] = word & 0xff;
                this.gr[0x21] = (word >> 8) & 0xff;
                this.gr[0x22] = (word >> 16) & 0xff;
                this.gr[0x23] = (word >> 24) & 0xff;
                break;
            case 0x0c:
                this.gr[0x24] = word & 0xff;
                this.gr[0x25] = (word >> 8) & 0xff;
                this.gr[0x26] = (word >> 16) & 0xff;
                this.gr[0x27] = (word >> 24) & 0xff;
                break;
            case 0x10:
                this.gr[0x28] = word & 0xff;
                this.gr[0x29] = (word >> 8) & 0xff;
                this.gr[0x2a] = (word >> 16) & 0xff;
                this.gr[0x2b] = (word >> 24) & 0xff;
                break;
            case 0x14:
                this.gr[0x2c] = word & 0xff;
                this.gr[0x2d] = (word >> 8) & 0xff;
                this.gr[0x2e] = (word >> 16) & 0xff;
                this.gr[0x2f] = (word >> 24) & 0xff;
                break;
            case 0x18:
                this.gr[0x30] = word & 0xff;
                this.gr[0x32] = (word >> 16) & 0xff;
                this.gr[0x33] = (word >> 24) & 0xff;
                break;
            case 0x40:
                this.gr[0x31] = word & 0xff;
                if ((word & 0xff) == 0x02) {
                    this.start_bitblt();
                }
                break;
        }
        return 0;
    }
    throw "CL-GD5440: Unhandled 32-bit write to 0x" + address.toString(16) + "\n";
};

//
// Read a 16-bit word from RAM.
//
clgd5440.prototype.pci_mem_read_word = function (address)
{
    if (((address & 0xff000000) >>> 0) == 0x81000000) { // Linear memory
        address &= CLGD5440_RAM_MASK & ~3;
        return this.ram[address] | (this.ram[address + 1] << 8) | (this.ram[address + 2] << 16) | (this.ram[address + 3] << 24);
    }
    throw "CL-GD5440: Unhandled 32-bit write to 0x" + address.toString(16) + "\n";
};

//
// Subset of PCI header for OS detection (just vendor and device ID)
//
clgd5440.prototype.pci_header = [
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
    0x00000000];

//
// Read a 32-bit word from RAM.
//
clgd5440.prototype.pci_mem_read_dword = function (address)
{
    if (((address & 0xc0000000) >>> 0) == 0xc0000000) {
        if ((address & 0x08000000) != 0) {  // Slot closer to the power supply if I remember right
            return this.pci_header[(address >> 2) & 0x0f];
        }
        if ((address & 0x10080000) != 0) {
            
        }
        if ((address & 0x20080000) != 0) {
            
        }
        return 0xffffffff;  /* Nothing here */
    }
    if (((address & 0xffff8000) >>> 0) == 0x800b8000) {
        switch (address & 0xfc) {
            default:
                throw "CL-GD5440: Unhandled byte read address 0x" + address.toString(16) + "\n";
            case 0x40:
                return this.gr[0x31];
        }
    }
    if (((address & 0xff000000) >>> 0) == 0x81000000) {
        address &= CLGD5440_RAM_MASK & ~3;
        return this.ram[address] | (this.ram[address + 1] << 8) | (this.ram[address + 2] << 16) | (this.ram[address + 3] << 24);
    }
    throw "CL-GD5440: Unhandled byte read address 0x" + address.toString(16) + "\n";
};

//
// Save the background under the cursor.
//
clgd5440.prototype.save_cursor = function() {
    var source;
    var y;
    var c;
    
    if ((this.sr[0x12] & 1) == 0)
        return;
    source = (this.cursor_y * 800 + this.cursor_x) * 2;
    for (y = 0; y < 32; y++) {
        source &= CLGD5440_RAM_MASK;
        if (source + 64 > CLGD5440_RAM_SIZE)
            continue;
        for (c = 0; c < 64; c++)
            this.cursor[y * 64 + c] = this.ram[source + c];
        source += 800 * 2;
    }
}

//
// Restore the background under the cursor.
//
clgd5440.prototype.restore_cursor = function() {
    var source;
    var y;
    var c;

    if ((this.sr[0x12] & 1) == 0)
        return;
    source = (this.cursor_y * 800 + this.cursor_x) * 2;
    for (y = 0; y < 32; y++) {
        source &= CLGD5440_RAM_MASK;
        if (source + 64 > CLGD5440_RAM_SIZE)
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
clgd5440.prototype.draw_cursor = function() {
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
        source = (CLGD5440_RAM_SIZE - 16384) + (this.sr[0x13] * 256) + y * 4;
        target = ((this.cursor_y + y) * 800 + this.cursor_x) * 2;
        if (target + 64 > CLGD5440_RAM_SIZE)
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
clgd5440.prototype.render = function () {
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
