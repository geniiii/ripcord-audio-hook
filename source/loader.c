#define READ_VOICE_PACKET(name) u64 name(void* _this, void* decrypted_data, size_t original_size, u8 a4, u16 a5, u32 a6, u32 encrypted_size)
typedef READ_VOICE_PACKET(ReadVoicePacketType);

static ReadVoicePacketType* read_voice_packet_orig;
static READ_VOICE_PACKET(ReadVoicePacketHook) {
	u8* data = (u8*) decrypted_data;
	if (data[0] == 0xBE && data[1] == 0xDE) {
		u16 size_in_dwords = data[3] | data[2] << 8;
		if (size_in_dwords > 1) {
			u32 size = size_in_dwords * 4 - 4;
			return read_voice_packet_orig(_this, data + size, original_size - size_in_dwords - 1, a4, a5, a6, encrypted_size);
		}
	}

	return read_voice_packet_orig(_this, decrypted_data, original_size, a4, a5, a6, encrypted_size);
}

static void LoadHooks() {
	MH_Initialize();

	MODULEINFO module_info;
	GetModuleInformation(GetCurrentProcess(), GetModuleHandleA("Ripcord.exe"), &module_info, sizeof module_info);
	MH_CreateHook((u8*) module_info.lpBaseOfDll + 0xD0DF0, (LPVOID) &ReadVoicePacketHook, (LPVOID*) &read_voice_packet_orig);
	MH_EnableHook(MH_ALL_HOOKS);
}
