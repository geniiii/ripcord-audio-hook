#define READ_VOICE_PACKET(name) u64 name(void* this, u8* data, size_t original_size, u8 a4, u16 a5, u32 a6, u32 encrypted_size)
typedef READ_VOICE_PACKET(ReadVoicePacketType);

static ReadVoicePacketType* read_voice_packet_orig;

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

static u32 LoadHooks() {
	if (MH_Initialize() != MH_OK) {
		ErrorMessage("Failed to initialize MinHook");
		return 0;
	}

	MODULEINFO module_info;
	GetModuleInformation(GetCurrentProcess(), GetModuleHandleA("Ripcord.exe"), &module_info, sizeof module_info);

	u32 result = 1;
	result &= CreateAndEnableHook((u8*) module_info.lpBaseOfDll, 0xD0DF0, (LPVOID) &ReadVoicePacketHook, (LPVOID*) &read_voice_packet_orig);

	return result;
}
