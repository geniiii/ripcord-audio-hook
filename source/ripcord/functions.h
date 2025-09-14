#define READ_VOICE_PACKET(name) u64 name(void* this, u8* data, u64 original_size, u8 a4, u16 a5, u32 a6, u32 encrypted_size)
typedef READ_VOICE_PACKET(ReadVoicePacketType);

#define ERF_MAP_FIND(name) u8 name(ErfMap* map, const char* key, u64 key_size, ErfMapAny* out)
typedef ERF_MAP_FIND(ErfMapFindType);

#define ERF_ARR_AT(name) ErfMapAny* name(ErfArr* this, ErfMapAny* result, i32 i)
typedef ERF_ARR_AT(ErfArrAtType);

#define UPDATEUSERGUILDPOSITIONS(name) void name(DisDbPrepared* comStmts, FlakeId userId, ErfArr* guildPosList)
typedef UPDATEUSERGUILDPOSITIONS(UpdateUserGuildPositionsType);

#define RIPSTMT_CONSTRUCTOR(name) void name(RipStmt* this, void* db, const char* sql, int* errcode)
typedef RIPSTMT_CONSTRUCTOR(RipStmtConstructorType);

#define RIPSTMT_DESTRUCTOR(name) void name(RipStmt* this)
typedef RIPSTMT_DESTRUCTOR(RipStmtDestructorType);

#define RIPSTMT_BIND_U64(name) void name(RipStmt* this, int index, u64 value)
typedef RIPSTMT_BIND_U64(RipStmtBindU64Type);

#define RIPSTMT_STEP(name) void name(RipStmt* this)
typedef RIPSTMT_STEP(RipStmtStepType);

#define RIPSTMT_RESET(name) void name(RipStmt* this)
typedef RIPSTMT_RESET(RipStmtResetType);

#define DISDBPREPARED_BEGINTX(name) void name(DisDbPrepared* this)
typedef DISDBPREPARED_BEGINTX(DisDbPreparedBegintxType);

#define DISDBPREPARED_ENDTX(name) void name(DisDbPrepared* this)
typedef DISDBPREPARED_ENDTX(DisDbPreparedEndtxType);


static ReadVoicePacketType* read_voice_packet;
static ErfMapFindType*      erf_map_find;
static ErfArrAtType*        erf_arr_at;
static UpdateUserGuildPositionsType* update_user_guild_positions;
static RipStmtConstructorType* ripstmt_constructor;
static RipStmtDestructorType* ripstmt_destructor;
static RipStmtBindU64Type* ripstmt_bind_u64;
static RipStmtStepType* ripstmt_step;
static RipStmtResetType* ripstmt_reset;
static DisDbPreparedBegintxType* disdbprepared_begintx;
static DisDbPreparedEndtxType* disdbprepared_endtx;
