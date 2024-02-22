#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <funchook.h>

typedef int64_t  i64;
typedef int32_t  i32;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#include "string8.c"

typedef struct {
    u8* data;
    u64 size_as_4;
} DisVoiceDatagram;
typedef struct {
    DisVoiceDatagram datagram;
    u32              _unk1;
    u32              _unk2;
    u32              _unk3;
    u16              _unk4;
    u8               _unk5;
    u8               _padding;
} SomeVoiceData;
typedef struct {
    void*          _unk1;
    SomeVoiceData* some_voice_data_array;
    SomeVoiceData* some_voice_data_array_unk;
    u8*            internal_buf;
    u64            buf_size;
    u64            buf_size_in_dwords;
} VoiceDataAccum;

enum {
    ErfMapAny_Num = 2,
};
enum {
    DisChannelType_Voice = 2,
    DisChannelType_Stage = 13,
};
typedef struct {
    u32 tag;
    u32 _unk;
    u32 num;
} ErfMapAny;

#define WRITE_DATAGRAM(name) i64 name(void* this, u8* data, i64 size, void* addr, u16 port)
typedef WRITE_DATAGRAM(WriteDatagramType);

#define READ_VOICE_DATA_PACKET(name) void name(VoiceDataAccum* accum, u32 a2, const u8* a3, char* a4, u64 a5)
typedef READ_VOICE_DATA_PACKET(ReadVoiceDataPacketType);

#define ERF_MAP_FIND(name) u8 name(void* map, const char* key, u64 key_size, ErfMapAny* out)
typedef ERF_MAP_FIND(ErfMapFindType);

static WriteDatagramType*       write_datagram_orig;
static ReadVoiceDataPacketType* read_voice_data_packet_orig;
static ErfMapFindType*          erf_map_find_orig;

WRITE_DATAGRAM(_ZN10QUdpSocket13writeDatagramEPKcxRK12QHostAddresst) {
    if (!write_datagram_orig) {
        write_datagram_orig = dlsym(RTLD_NEXT, "_ZN10QUdpSocket13writeDatagramEPKcxRK12QHostAddresst");
    }

    // NOTE(geni): Ripcord appears to send what is likely an old version of this packet, which *only some* Discord servers seem to respond to.
    //             See here: https://discord.com/developers/docs/topics/voice-connections#ip-discovery
    if (size == 70) {
        puts("Hooked writeDatagram");
        u32 ssrc         = *((u32*) data);
        data[0]          = 0;
        data[1]          = 1;
        data[2]          = 0;
        data[3]          = 70;
        ((u32*) data)[1] = ssrc;
        return write_datagram_orig(this, data, 74, addr, port);
    }

    return write_datagram_orig(this, data, size, addr, port);
}

READ_VOICE_DATA_PACKET(ReadVoiceDataPacketHook) {
    read_voice_data_packet_orig(accum, a2, a3, a4, a5);

    if (accum == NULL || accum->some_voice_data_array == NULL) {
        return;
    }

    DisVoiceDatagram* dg = &accum->some_voice_data_array[-1].datagram;
    if (dg->data[0] == 0xBE && dg->data[1] == 0xDE) {
        u16 size_in_dwords = dg->data[3] | dg->data[2] << 8;
        if (size_in_dwords > 1) {
            --size_in_dwords;
            dg->data += size_in_dwords * 4;
            dg->size_as_4 -= size_in_dwords;
        }
    }
}

ERF_MAP_FIND(ErfMapFindHook) {
    u8 result = erf_map_find_orig(map, key, key_size, out);

    if (result && key_size == 4 &&
        out->tag == ErfMapAny_Num && memcmp(key, "type", 4) == 0 &&
        out->num == DisChannelType_Stage) {
        out->num = DisChannelType_Voice;
    }

    return result;
}

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

static void PatchString(u8* addr, String8 value) {
    printf("Original %p: %s\n", addr, (char*) addr);
    u32 page_size = sysconf(_SC_PAGE_SIZE);
    u8* page_addr = (u8*) ((u64) addr & ~(page_size - 1));
    if (mprotect(page_addr, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) == -1) {
        puts("mprotect failed");
    }
    memcpy(addr, value.data, value.size);
    printf("Patched %p: %.*s\n", addr, value.size, (char*) addr);
}

static __attribute__((constructor)) void OnAttach(void) {
    u8* base = (u8*) 0x400000;

    u8* receive_udp = base + 0x155870;
    PatchByte(receive_udp + 0x102, 74);
    PatchByte(receive_udp + 0x110, 8);
    PatchByte(receive_udp + 0x172, 72);
    puts("Patched bytes");

    PatchString(base + 0x4E997D, S8Lit("cdn.discordapp.com/\0\0\0"));
    puts("Patched media URL");

    read_voice_data_packet_orig = (ReadVoiceDataPacketType*) (base + 0x1557B0);
    erf_map_find_orig           = (ErfMapFindType*) (base + 0x1216B0);
    funchook_t* funchook        = funchook_create();

    i32 err = funchook_prepare(funchook, (void**) &read_voice_data_packet_orig, ReadVoiceDataPacketHook);
    if (err != 0) {
        puts("Failed to prepare DisClientImpl::readVoiceDataPacket hook");
        return;
    }
    err = funchook_prepare(funchook, (void**) &erf_map_find_orig, ErfMapFindHook);
    if (err != 0) {
        puts("Failed to prepare Erf::Map::Find hook");
        return;
    }
    err = funchook_install(funchook, 0);
    if (err != 0) {
        puts("Failed to install hooks");
        return;
    }
    puts("Installed hooks");
}
