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

#define READ_VOICE_PACKET(name) u64 name(void* this, u8* data, u64 original_size, u8 a4, u16 a5, u32 a6, u32 encrypted_size)
typedef READ_VOICE_PACKET(ReadVoicePacketType);

#define WRITE_DATAGRAM(name) i64 name(void* this, u8* data, i64 size, void* addr, u16 port)
typedef WRITE_DATAGRAM(WriteDatagramType);

#define ERF_MAP_FIND(name) u8 name(void* map, const char* key, u64 key_size, ErfMapAny* out)
typedef ERF_MAP_FIND(ErfMapFindType);

static ReadVoicePacketType* read_voice_packet_orig;
static WriteDatagramType*   write_datagram_orig;
static ErfMapFindType*      erf_map_find_orig;

static WRITE_DATAGRAM(WriteDatagramHook) {
    // NOTE(geni): Ripcord appears to send what is likely an old version of this packet, which *only some* Discord servers seem to respond to.
    //             See here: https://discord.com/developers/docs/topics/voice-connections#ip-discovery
    if (size == 70) {
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

static READ_VOICE_PACKET(ReadVoicePacketHook) {
    if (data[0] == 0xBE && data[1] == 0xDE) {
        u16 size_in_dwords = data[3] | data[2] << 8;
        if (size_in_dwords > 1) {
            --size_in_dwords;
            return read_voice_packet_orig(this, data + size_in_dwords * 4, original_size - size_in_dwords, a4, a5, a6, encrypted_size);
        }
    }

    return read_voice_packet_orig(this, data, original_size, a4, a5, a6, encrypted_size);
}

static ERF_MAP_FIND(ErfMapFindHook) {
    u8 result = erf_map_find_orig(map, key, key_size, out);

    if (result && key_size == 4 &&
        out->tag == ErfMapAny_Num && memcmp(key, "type", 4) == 0 &&
        out->num == DisChannelType_Stage) {
        out->num = DisChannelType_Voice;
    }

    return result;
}

static u32 CreateAndEnableHook(u8* base, u64 ptr, void* hook, void** orig) {
    static char buffer[4096];

    MH_STATUS status;
    if ((status = MH_CreateHook(base + ptr, hook, orig)) != MH_OK) {
        sprintf(buffer, "Failed to create hook at 0x%llX\nError code: %d", ptr, status);
        ErrorMessage(buffer);
        return 0;
    }
    if ((status = MH_EnableHook(base + ptr)) != MH_OK) {
        sprintf(buffer, "Failed to enable hook at 0x%llX\nError code: %d", ptr, status);
        ErrorMessage(buffer);
        return 0;
    }

    return 1;
}

static void PatchByte(u8* base, u64 ptr, u8 new) {
    DWORD old_protect;
    u8*   addr = base + ptr;
    VirtualProtect(addr, 1, PAGE_EXECUTE_READWRITE, &old_protect);
    *addr = new;
    VirtualProtect(addr, 1, old_protect, &old_protect);
}

static u32 LoadHooks() {
    if (MH_Initialize() != MH_OK) {
        ErrorMessage("Failed to initialize MinHook");
        return 0;
    }

    MODULEINFO module_info;
    GetModuleInformation(GetCurrentProcess(), GetModuleHandleA("Ripcord.exe"), &module_info, sizeof module_info);
    u8* rip_base = (u8*) module_info.lpBaseOfDll;

    for (u32 i = 0; i < sizeof SUPPORTED_VERSION; ++i) {
        if (rip_base[0x3BAB70 + i] != SUPPORTED_VERSION[i]) {
            ErrorMessage("Unsupported Ripcord version (expected " SUPPORTED_VERSION ")");
            return 0;
        }
    }

    PatchByte(rip_base, 0xDE8D8, 74);
    PatchByte(rip_base, 0xDE8EA, 8);
    PatchByte(rip_base, 0xDE90C, 8);
    PatchByte(rip_base, 0xDE936, 72);

    u32 result = 1;
    result &= CreateAndEnableHook(rip_base, 0xD0DF0, (LPVOID) &ReadVoicePacketHook, (LPVOID*) &read_voice_packet_orig);
    u64 write_datagram_addr = (u64) GetProcAddress(GetModuleHandleA("Qt5Network.dll"), "?writeDatagram@QUdpSocket@@QEAA_JPEBD_JAEBVQHostAddress@@G@Z");
    result &= CreateAndEnableHook(0, write_datagram_addr, (LPVOID) &WriteDatagramHook, (LPVOID*) &write_datagram_orig);
    result &= CreateAndEnableHook(rip_base, 0xB9690, (LPVOID) &ErfMapFindHook, (LPVOID*) &erf_map_find_orig);

    return result;
}
