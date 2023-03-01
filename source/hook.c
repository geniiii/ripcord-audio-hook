#define READ_VOICE_PACKET(name) u64 name(void* this, u8* data, size_t original_size, u8 a4, u16 a5, u32 a6, u32 encrypted_size)
typedef READ_VOICE_PACKET(ReadVoicePacketType);

#define WRITE_DATAGRAM(name) i64 name(void* this, u8* data, i64 size, void* addr, u16 port)
typedef WRITE_DATAGRAM(WriteDatagramType);

static ReadVoicePacketType* read_voice_packet_orig;
static WriteDatagramType* write_datagram_orig;

static WRITE_DATAGRAM(WriteDatagramHook) {
    // NOTE(geni): Ripcord appears to send what is likely an old version of this packet, which *only some* Discord servers seem to respond to.
    //             See here: https://discord.com/developers/docs/topics/voice-connections#ip-discovery
    if (size == 70) {
        u32 ssrc = *((u32*) data);
        data[0] = 0;
        data[1] = 1;
        data[2] = 0;
        data[3] = 70;
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
    u8* addr = (u8*) base + ptr;
    VirtualProtect(base + ptr, 1, PAGE_EXECUTE_READWRITE, &old_protect);
    base[ptr] = new;
    VirtualProtect(addr, 1, old_protect, &old_protect);
}

static u32 LoadHooks() {
	if (MH_Initialize() != MH_OK) {
		ErrorMessage("Failed to initialize MinHook");
		return 0;
	}

	MODULEINFO module_info;
	GetModuleInformation(GetCurrentProcess(), GetModuleHandleA("Ripcord.exe"), &module_info, sizeof module_info);

	for (u32 i = 0; i < sizeof SUPPORTED_VERSION; ++i) {
		if (*((u8*) module_info.lpBaseOfDll + 0x3BAB70 + i) != SUPPORTED_VERSION[i]) {
			ErrorMessage("Unsupported Ripcord version (expected " SUPPORTED_VERSION ")");
			return 0;
		}
	}

    PatchByte(module_info.lpBaseOfDll, 0xDE8D8, 74);
    PatchByte(module_info.lpBaseOfDll, 0xDE8EA, 8);
    PatchByte(module_info.lpBaseOfDll, 0xDE90C, 8);
    PatchByte(module_info.lpBaseOfDll, 0xDE936, 72);

	u32 result = 1;
	result &= CreateAndEnableHook(module_info.lpBaseOfDll, 0xD0DF0, (LPVOID) &ReadVoicePacketHook, (LPVOID*) &read_voice_packet_orig);
    u8* write_datagram_ptr = (u8*) GetProcAddress(GetModuleHandleA("Qt5Network.dll"), "?writeDatagram@QUdpSocket@@QEAA_JPEBD_JAEBVQHostAddress@@G@Z");
    result &= CreateAndEnableHook(0, (u64) write_datagram_ptr, (LPVOID) &WriteDatagramHook, (LPVOID*) &write_datagram_orig);

	return result;
}
