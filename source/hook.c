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
        return write_datagram(this, data, 74, addr, port);
    }

    return write_datagram(this, data, size, addr, port);
}

static READ_VOICE_PACKET(ReadVoicePacketHook) {
    if (data[0] == 0xBE && data[1] == 0xDE) {
        u16 size_in_dwords = data[3] | data[2] << 8;
        if (size_in_dwords > 1) {
            --size_in_dwords;
            return read_voice_packet(this, data + size_in_dwords * 4, original_size - size_in_dwords, a4, a5, a6, encrypted_size);
        }
    }

    return read_voice_packet(this, data, original_size, a4, a5, a6, encrypted_size);
}

// NOTE(geni): The current maximum on Discord's side is 200, so we'll be fine
static u64 guilds[512];
static u32 guilds_count;

static UPDATEUSERGUILDPOSITIONS(UpdateUserGuildPositionsHook) {
    RipStmt orderQ;
    RipStmt deleteQ;

    if (comStmts) {
        disdbprepared_begintx(comStmts);
    }
    ripstmt_constructor(&deleteQ, comStmts->db, "\ndelete from user_guild_position\nwhere user_id = ?\n", 0i64);
    ripstmt_bind_u64(&deleteQ, 1, userId.u);
    ripstmt_step(&deleteQ);
    ripstmt_reset(&deleteQ);
    ripstmt_destructor(&deleteQ);
    ripstmt_constructor(
        &orderQ,
        comStmts->db,
        "\nreplace into user_guild_position\n(user_id, guild_id, position)\nvalues (?, ?, ?)\n",
        0i64);
    for (u32 i = 0; i < guilds_count; ++i) {
        ripstmt_bind_u64(&orderQ, 1, userId.u);
        ripstmt_bind_u64(&orderQ, 2, guilds[i]);
        ripstmt_bind_u64(&orderQ, 3, i);
        ripstmt_step(&orderQ);
        ripstmt_reset(&orderQ);
    }
    ripstmt_destructor(&orderQ);
    disdbprepared_endtx(comStmts);
}

static ERF_MAP_FIND(ErfMapFindHook) {
    u8 result = erf_map_find(map, key, key_size, out);

    if (result && key_size == 4 &&
        out->tag == ErfTag_Int32 && memcmp(key, "type", 4) == 0 &&
        out->int32 == DisChannelType_GuildVoiceStage) {
        out->int32 = DisChannelType_GuildVoice;
    } else {
        ErfMapAny new_out = {.tag = ErfTag_Nil};
        // NOTE(geni): We can probably just check return address instead and it would probably be faster
        if (map && key_size == 15 &&
            memcmp(key, "guild_positions", 15) == 0 &&
            erf_map_find(map, "guild_folders", 13, &new_out) &&
            new_out.tag == ErfTag_Arr) {
            ErfArr arr          = new_out.arr;
            ErfMap guild_folder = {0};
            for (i32 i = 0; i < arr.count; ++i) {
                erf_arr_at(&arr, &new_out, i);
                if (new_out.tag != ErfTag_Map) {
                    continue;
                }
                guild_folder = new_out.map;

                if (erf_map_find(&guild_folder, "guild_ids", 9, &new_out) && new_out.tag == ErfTag_Arr) {
                    ErfArr guild_ids = new_out.arr;
                    for (i32 j = 0; j < guild_ids.count; ++j) {
                        erf_arr_at(&guild_ids, &new_out, j);
                        if (new_out.tag == ErfTag_Uint64) {
                            u64 guild_id           = new_out.uint64;
                            guilds[guilds_count++] = guild_id;
                        }
                    }
                }
            }
        }
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

static void PatchString(u8* base, u64 ptr, String8 new) {
    DWORD old_protect;
    u8*   addr = base + ptr;
    VirtualProtect(addr, new.size, PAGE_EXECUTE_READWRITE, &old_protect);
    CopyMemory(addr, new.data, new.size);
    VirtualProtect(addr, new.size, old_protect, &old_protect);
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

    // NOTE(geni): Fixes image previews not loading. Thanks u130b8!
    PatchString(rip_base, 0x3E63F8, S8Lit("cdn.discordapp.com/\0\0\0"));

    // NOTE(geni): Discard invalid map keys instead of aborting. Thanks @muffinlord on Discord!
    PatchByte(rip_base, 0xB9BBF, 0xEB);
    PatchByte(rip_base, 0xB9BC0, 0x80);

    // NOTE(geni): Disable gateway port splitting
    PatchByte(rip_base, 0xD677E, 0x10);

    u32 result = 1;
    result &= CreateAndEnableHook(rip_base, 0xD0DF0, (LPVOID) &ReadVoicePacketHook, (LPVOID*) &read_voice_packet);
    u64 write_datagram_addr = (u64) GetProcAddress(GetModuleHandleA("Qt5Network.dll"), "?writeDatagram@QUdpSocket@@QEAA_JPEBD_JAEBVQHostAddress@@G@Z");
    result &= CreateAndEnableHook(0, write_datagram_addr, (LPVOID) &WriteDatagramHook, (LPVOID*) &write_datagram);
    result &= CreateAndEnableHook(rip_base, 0xB9690, (LPVOID) &ErfMapFindHook, (LPVOID*) &erf_map_find);
    result &= CreateAndEnableHook(rip_base, 0xF9750, (LPVOID) &UpdateUserGuildPositionsHook, (LPVOID*) &update_user_guild_positions);

    disdbprepared_begintx = (DisDbPreparedBegintxType*) (rip_base + 0xF62E0);
    disdbprepared_endtx   = (DisDbPreparedEndtxType*) (rip_base + 0xF7070);
    ripstmt_constructor   = (RipStmtConstructorType*) (rip_base + 0x2110);
    ripstmt_destructor    = (RipStmtDestructorType*) (rip_base + 0x2230);
    ripstmt_bind_u64      = (RipStmtBindU64Type*) (rip_base + 0x25D0);
    ripstmt_step          = (RipStmtStepType*) (rip_base + 0x3440);
    ripstmt_reset         = (RipStmtResetType*) (rip_base + 0x3420);
    erf_arr_at            = (ErfArrAtType*) (rip_base + 0xD0EE0);

    return result;
}
