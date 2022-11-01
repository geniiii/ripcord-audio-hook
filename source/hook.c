#define READ_VOICE_PACKET(name) u64 name(void* this, void* decrypted_data, size_t original_size, u8 a4, u16 a5, u32 a6, u32 encrypted_size)
typedef READ_VOICE_PACKET(ReadVoicePacketType);

static ReadVoicePacketType* read_voice_packet_orig;

static READ_VOICE_PACKET(ReadVoicePacketHook) {
	u8* data = (u8*) decrypted_data;
	if (data[0] == 0xBE && data[1] == 0xDE) {
		u16 size_in_dwords = data[3] | data[2] << 8;
		if (size_in_dwords > 1) {
			--size_in_dwords;
			return read_voice_packet_orig(this, data + size_in_dwords * 4, original_size - size_in_dwords, a4, a5, a6, encrypted_size);
		}
	}
	
	return read_voice_packet_orig(this, decrypted_data, original_size, a4, a5, a6, encrypted_size);
}

static inline void CreateAndEnableHook(void* ptr, void* hook, void** orig) {
	MH_CreateHook(ptr, hook, orig);
	MH_EnableHook(ptr);
}

static void LoadHooks() {
	MH_Initialize();

	MODULEINFO module_info;
	GetModuleInformation(GetCurrentProcess(), GetModuleHandleA("Ripcord.exe"), &module_info, sizeof module_info);
	CreateAndEnableHook((u8*) module_info.lpBaseOfDll + 0xD0DF0, (LPVOID) &ReadVoicePacketHook, (LPVOID*) &read_voice_packet_orig);
}
