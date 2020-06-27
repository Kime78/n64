#include "pif.h"
#include "n64bus.h"

void pif_rom_execute(n64_system_t* system) {
    system->cpu.gpr[0] = 0;
    system->cpu.gpr[1] = 0;
    system->cpu.gpr[2] = 0;
    system->cpu.gpr[3] = 0;
    system->cpu.gpr[4] = 0;
    system->cpu.gpr[5] = 0;
    system->cpu.gpr[6] = 0;
    system->cpu.gpr[7] = 0;
    system->cpu.gpr[8] = 0;
    system->cpu.gpr[9] = 0;
    system->cpu.gpr[10] = 0;
    system->cpu.gpr[11] = 0;
    system->cpu.gpr[12] = 0;
    system->cpu.gpr[13] = 0;
    system->cpu.gpr[14] = 0;
    system->cpu.gpr[15] = 0;
    system->cpu.gpr[16] = 0;
    system->cpu.gpr[17] = 0;
    system->cpu.gpr[18] = 0;
    system->cpu.gpr[19] = 0;
    system->cpu.gpr[20] = 0x1;
    system->cpu.gpr[21] = 0;
    system->cpu.gpr[22] = 0x3F;
    system->cpu.gpr[23] = 0;
    system->cpu.gpr[24] = 0;
    system->cpu.gpr[25] = 0;
    system->cpu.gpr[26] = 0;
    system->cpu.gpr[27] = 0;
    system->cpu.gpr[28] = 0;
    system->cpu.gpr[29] = 0xA4001FF0;
    system->cpu.gpr[30] = 0;
    system->cpu.gpr[31] = 0;

    system->cpu.cp0.index        = 0;
    system->cpu.cp0.random       = 0x0000001F;
    system->cpu.cp0.entry_lo0    = 0;
    system->cpu.cp0.entry_lo1    = 0;
    system->cpu.cp0.context      = 0;
    system->cpu.cp0.page_mask    = 0;
    system->cpu.cp0.wired        = 0;
    system->cpu.cp0.r7           = 0;
    system->cpu.cp0.bad_vaddr    = 0;
    system->cpu.cp0.count        = 0;
    system->cpu.cp0.entry_hi     = 0;
    system->cpu.cp0.compare      = 0;
    system->cpu.cp0.status.raw   = 0x70400004;
    system->cpu.cp0.cause.raw    = 0;
    system->cpu.cp0.EPC          = 0;
    system->cpu.cp0.PRId         = 0x00000B00;
    system->cpu.cp0.config       = 0x0006E463;
    system->cpu.cp0.lladdr       = 0;
    system->cpu.cp0.watch_lo     = 0;
    system->cpu.cp0.watch_hi     = 0;
    system->cpu.cp0.x_context    = 0;
    system->cpu.cp0.r21          = 0;
    system->cpu.cp0.r22          = 0;
    system->cpu.cp0.r23          = 0;
    system->cpu.cp0.r24          = 0;
    system->cpu.cp0.r25          = 0;
    system->cpu.cp0.parity_error = 0;
    system->cpu.cp0.cache_error  = 0;
    system->cpu.cp0.tag_lo       = 0;
    system->cpu.cp0.tag_hi       = 0;
    system->cpu.cp0.error_epc    = 0;
    system->cpu.cp0.r31          = 0;

    logwarn("CP0 status: ie:  %d", system->cpu.cp0.status.ie)
    logwarn("CP0 status: exl: %d", system->cpu.cp0.status.exl)
    logwarn("CP0 status: erl: %d", system->cpu.cp0.status.erl)
    logwarn("CP0 status: ksu: %d", system->cpu.cp0.status.ksu)
    logwarn("CP0 status: ux:  %d", system->cpu.cp0.status.ux)
    logwarn("CP0 status: sx:  %d", system->cpu.cp0.status.sx)
    logwarn("CP0 status: kx:  %d", system->cpu.cp0.status.kx)
    logwarn("CP0 status: im:  %d", system->cpu.cp0.status.im)
    logwarn("CP0 status: ds:  %d", system->cpu.cp0.status.ds)
    logwarn("CP0 status: re:  %d", system->cpu.cp0.status.re)
    logwarn("CP0 status: fr:  %d", system->cpu.cp0.status.fr)
    logwarn("CP0 status: rp:  %d", system->cpu.cp0.status.rp)
    logwarn("CP0 status: cu0: %d", system->cpu.cp0.status.cu0)
    logwarn("CP0 status: cu1: %d", system->cpu.cp0.status.cu1)
    logwarn("CP0 status: cu2: %d", system->cpu.cp0.status.cu2)
    logwarn("CP0 status: cu3: %d", system->cpu.cp0.status.cu3)

    //n64_write_word(system, 0x04300004, 0x01010101);

    // Copy the first 0x1000 bytes of the cartridge to 0xA4000000

    word src_ptr  = 0xB0000000;
    word dest_ptr = 0xA4000000;

    for (int i = 0; i < 0x1000; i++) {
        word src_address = src_ptr + i;
        word dest_address = dest_ptr + i;
        byte src = n64_read_byte(system, src_address);
        n64_write_byte(system, dest_address, src);

        logtrace("PIF: Copied 0x%02X from 0x%08X ==> 0x%08X", src, src_address, dest_address)
    }

    system->cpu.pc = 0xA4000040;
}

#define PIF_COMMAND_CONTROLLER_ID 0x00
#define PIF_COMMAND_READ_BUTTONS  0x01
#define PIF_COMMAND_MEMPACK_READ  0x02
#define PIF_COMMAND_MEMPACK_WRITE 0x03
#define PIF_COMMAND_EEPROM_READ   0x04
#define PIF_COMMAND_EEPROM_WRITE  0x05
#define PIF_COMMAND_RESET         0xFF

void pif_command(n64_system_t* system, sbyte cmdlen, byte reslen, int* index, int* channel) {
    if (cmdlen == 0) {
        (*channel)++;
        return;
    }
    byte command = system->mem.pif_ram[(*index)];
    (*index)++;
    switch (command) {
        case PIF_COMMAND_CONTROLLER_ID:
            unimplemented(cmdlen != 1, "Controller ID with cmdlen != 1")
            unimplemented(reslen != 3, "Controller ID with reslen != 3")
            system->mem.pif_ram[(*index)++] = 0x05;
            system->mem.pif_ram[(*index)++] = 0x00;
            system->mem.pif_ram[(*index)++] = 0x02; // No controller pak plugged in.
            break;
        case PIF_COMMAND_READ_BUTTONS:
            unimplemented(cmdlen != 1, "Read button values with cmdlen != 1")
            unimplemented(reslen != 4, "Read button values with reslen != 4")
            byte bytes[4];
            if (*channel < 4) {
                bytes[0] = system->si.controllers[*channel].byte1;
                bytes[1] = system->si.controllers[*channel].byte2;
                bytes[2] = system->si.controllers[*channel].joy_x;
                bytes[3] = system->si.controllers[*channel].joy_y;
            }
            system->mem.pif_ram[(*index)++] = bytes[0];
            system->mem.pif_ram[(*index)++] = bytes[1];
            system->mem.pif_ram[(*index)++] = bytes[2];
            system->mem.pif_ram[(*index)++] = bytes[3];
            (*channel)++;
            break;
        case PIF_COMMAND_MEMPACK_READ:
            logfatal("PIF_COMMAND_MEMPACK_READ")
            break;
        case PIF_COMMAND_MEMPACK_WRITE:
            unimplemented(cmdlen != 35, "Mempack write with cmdlen != 35")
            unimplemented(reslen != 1, "Mempack write with reslen != 1")
            (*index) += 36; // NOOP
            break;
        case PIF_COMMAND_EEPROM_READ:
            logfatal("PIF_COMMAND_EEPROM_READ")
        case PIF_COMMAND_EEPROM_WRITE:
            logfatal("PIF_COMMAND_EEPROM_WRITE")
        case PIF_COMMAND_RESET:
            (*index) += reslen;
            break;
        default:
            logfatal("Unknown PIF command: %d", command)
    }
}

void process_pif_command(n64_system_t* system) {
    byte control = system->mem.pif_ram[63];
    if (control == 1) {
        int i = 0;
        int channel = 0;
        while (i < 63) {
            sbyte t = system->mem.pif_ram[i++];
            if (t >= 0) {
                // TODO: should R be signed?
                byte r = system->mem.pif_ram[i++]; // TODO: out of bounds access possible on invalid data
                pif_command(system, t, r, &i, &channel);
            }
        }
    }
}


void update_button(n64_system_t* system, int controller, n64_button_t button, bool held) {
    switch(button) {
        case A:
            loginfo("Button A is now: %d\n", held);
            system->si.controllers[controller].a = held;
            break;
        case B:
            loginfo("Button B is now: %d\n", held);
            system->si.controllers[controller].b = held;
            break;
    }
}