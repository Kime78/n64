#ifndef N64_N64BUS_H
#define N64_N64BUS_H

#include <util.h>
#include <system/n64system.h>
#include "addresses.h"

bool tlb_probe(dword vaddr, word* paddr, int* entry_number, cp0_t* cp0);
bool tlb_probe_64(dword vaddr, word* paddr, int* entry_number, cp0_t* cp0);

#define REGION_XKUSEG 0x0000000000000000 ... 0x000000FFFFFFFFFF
#define REGION_XBAD1  0x0000010000000000 ... 0x3FFFFFFFFFFFFFFF
#define REGION_XKSSEG 0x4000000000000000 ... 0x400000FFFFFFFFFF
#define REGION_XBAD2  0x4000010000000000 ... 0x7FFFFFFFFFFFFFFF
#define REGION_XKPHYS 0x8000000000000000 ... 0xBFFFFFFFFFFFFFFF
#define REGION_XKSEG  0xC000000000000000 ... 0xC00000FF7FFFFFFF
#define REGION_XBAD3  0xC00000FF80000000 ... 0xFFFFFFFF7FFFFFFF
#define REGION_CKSEG0 0xFFFFFFFF80000000 ... 0xFFFFFFFF9FFFFFFF
#define REGION_CKSEG1 0xFFFFFFFFA0000000 ... 0xFFFFFFFFBFFFFFFF
#define REGION_CKSSEG 0xFFFFFFFFC0000000 ... 0xFFFFFFFFDFFFFFFF
#define REGION_CKSEG3 0xFFFFFFFFE0000000 ... 0xFFFFFFFFFFFFFFFF

INLINE word resolve_virtual_address_32bit(word address, cp0_t* cp0) {
    word physical;
    switch (address >> 29) {
        // KSEG0
        case 0x4:
            // Unmapped translation. Subtract the base address of the space to get the physical address.
            physical = address - SVREGION_KSEG0;
            logtrace("KSEG0: Translated 0x%08X to 0x%08X", address, physical);
            break;
        // KSEG1
        case 0x5:
            // Unmapped translation. Subtract the base address of the space to get the physical address.
            physical = address - SVREGION_KSEG1;
            logtrace("KSEG1: Translated 0x%08X to 0x%08X", address, physical);
            break;
        // KUSEG
        case 0x0:
        case 0x1:
        case 0x2:
        case 0x3: {
            if (!tlb_probe(address, &physical, NULL, cp0)) {
                logfatal("Unimplemented: page miss translating virtual address 0x%08X in VREGION_KUSEG", address);
            }
            break;
        }
        // KSSEG
        case 0x6:
            logfatal("Unimplemented: translating virtual address 0x%08X in VREGION_KSSEG", address);
        // KSEG3
        case 0x7:
            if (!tlb_probe(address, &physical, NULL, cp0)) {
                logfatal("Unimplemented: page miss translating virtual address 0x%08X in VREGION_KSEG3", address);
            }
            break;
        default:
            logfatal("PANIC! should never end up here.");
    }
    return physical;
}

INLINE word resolve_virtual_address_64bit(dword address, cp0_t* cp0) {
    word physical;
    switch (address) {
        case REGION_XKUSEG:
            if (!tlb_probe_64(address, &physical, NULL, cp0)) {
                logfatal("Unimplemented: page miss translating virtual address 0x%016lX in REGION_XKUSEG", address);
            }
            break;
    case REGION_XKSSEG:
            logfatal("Resolving virtual address 0x%016lX (REGION_XKSSEG) in 64 bit mode", address);
        case REGION_XKPHYS: {
            if (!cp0->kernel_mode) {
                logfatal("Access to XKPHYS address 0x%016lX when outside kernel mode!", address);
            }
            byte high_two_bits = (address >> 62) & 0b11;
            if (high_two_bits != 0b10) {
                logfatal("Access to XKPHYS address 0x%016lX with high two bits != 0b10!", address);
            }
            byte subsegment = (address >> 59) & 0b11;
            bool cached = subsegment != 2;
            if (cached) {
                logwarn("Resolving virtual address in cached XKPHYS subsegment %d", subsegment);
            }
            // If any bits in the range of 58:32 are set, the address is invalid.
            bool valid = (address & 0x07FFFFFF00000000) == 0;
            if (!valid) {
                logfatal("Invalid XKPHYS address 0x%016lX! bits in the range of 58:32 are set.", address);
            }
            physical = address & 0xFFFFFFFF;

            logwarn("XKPHYS: Translated 0x%016lX to 0x%08X", address, physical);
            break;
        }
        case REGION_XKSEG:
            logfatal("Resolving virtual address 0x%016lX (REGION_XKSEG) in 64 bit mode", address);
        case REGION_CKSEG0:
            // Identical to kseg0 in 32 bit mode.
            // Unmapped translation. Subtract the base address of the space to get the physical address.
            physical = address - SVREGION_KSEG0; // Implies cutting off the high 32 bits
            logtrace("CKSEG0: Translated 0x%016lX to 0x%08X", address, physical);
            break;
        case REGION_CKSEG1:
            // Identical to kseg1 in 32 bit mode.
            // Unmapped translation. Subtract the base address of the space to get the physical address.
            physical = address - SVREGION_KSEG1; // Implies cutting off the high 32 bits
            logtrace("KSEG1: Translated 0x%016lX to 0x%08X", address, physical);
            break;
        case REGION_CKSSEG:
            logfatal("Resolving virtual address 0x%016lX (REGION_CKSSEG) in 64 bit mode", address);
        case REGION_CKSEG3:
            logfatal("Resolving virtual address 0x%016lX (REGION_CKSEG3) in 64 bit mode", address);

        case REGION_XBAD1:
        case REGION_XBAD2:
        case REGION_XBAD3:
            logfatal("Resolving BAD virtual address 0x%016lX in 64 bit mode", address);
        default:
            logfatal("Resolving virtual address 0x%016lX in 64 bit mode", address);
    }

    return physical;
}

INLINE word resolve_virtual_address(dword address, cp0_t* cp0) {
    if (cp0->is_64bit_addressing) {
        return resolve_virtual_address_64bit(address, cp0);
    } else {
        return resolve_virtual_address_32bit(address, cp0);
    }
}


void n64_write_dword(n64_system_t* system, word address, dword value);
dword n64_read_dword(n64_system_t* system, word address);

void n64_write_word(n64_system_t* system, word address, word value);
word n64_read_word(word address);
word n64_read_physical_word(word address);

void n64_write_half(n64_system_t* system, word address, half value);
half n64_read_half(n64_system_t* system, word address);

void n64_write_byte(n64_system_t* system, word address, byte value);
byte n64_read_byte(n64_system_t* system, word address);

#endif //N64_N64BUS_H
