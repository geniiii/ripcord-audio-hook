#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <dlfcn.h>

typedef int64_t  i64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

static void PatchByte(u8* addr, u8 value) {
    printf("Original %p: %x\n", addr, *addr);
    u32 page_size = sysconf(_SC_PAGE_SIZE);
    u8* page_addr = (u8*) ((u64) addr & ~(page_size - 1));
    if (mprotect(page_addr, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) == -1) {
        puts("mprotect failed");
    }
    *addr = value;
    printf("Patched %p: %x\n", addr, value);
}

static __attribute__((constructor)) void OnAttach(void) {
    u8* receive_udp = (u8*) (0x400000 + 0x155870);
    PatchByte(receive_udp + 0x102, 74);
    PatchByte(receive_udp + 0x110, 8);
    PatchByte(receive_udp + 0x172, 72);
    puts("Patched bytes");
}

#define WRITE_DATAGRAM(name) i64 name(void* this, u8* data, i64 size, void* addr, u16 port)
typedef WRITE_DATAGRAM(WriteDatagramType);
static WriteDatagramType* write_datagram_orig;
WRITE_DATAGRAM(_ZN10QUdpSocket13writeDatagramEPKcxRK12QHostAddresst) {
    if (!write_datagram_orig) {
        write_datagram_orig = dlsym(RTLD_NEXT, "_ZN10QUdpSocket13writeDatagramEPKcxRK12QHostAddresst");
    }

    // NOTE(geni): Ripcord appears to send what is likely an old version of this packet, which *only some* Discord servers seem to respond to.
    //             See here: https://discord.com/developers/docs/topics/voice-connections#ip-discovery
    if (size == 70) {
        u32 ssrc         = *((u32*) data);
        data[0]          = 0;
        data[1]          = 1;
        data[2]          = 0;
        data[3]          = 70;
        ((u32*) data)[1] = ssrc;
        puts("Hooked writeDatagram");
        return write_datagram_orig(this, data, 74, addr, port);
    }

    return write_datagram_orig(this, data, size, addr, port);
}
