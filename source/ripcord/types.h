typedef enum {
    ErfTag_Nil,
    ErfTag_Bool,
    ErfTag_Int32,
    ErfTag_Int64,
    ErfTag_Uint64,
    ErfTag_Float64,
    ErfTag_Str,
    ErfTag_Arr,
    ErfTag_Map,
} ErfTag;

typedef union {
    u64 u;
    i64 i;
} FlakeId;

typedef enum {
    DisChannelType_Unknown,
    DisChannelType_GuildText,
    DisChannelType_GuildVoice,
    DisChannelType_GuildCategory,
    DisChannelType_PrivatePair,
    DisChannelType_PrivateGroup,

    DisChannelType_GuildVoiceStage = 13,
} DisChannelType;

__declspec(align(8)) typedef struct {
    i32         length;
    const char* data;
} ErfStr;

__declspec(align(8)) typedef struct {
    void* addr;
    i32   count;
} ErfMap;

__declspec(align(8)) typedef struct {
    void* addr;
    i32   count;
} ErfArr;

typedef struct {
    void* sqlite3_stmt;
} RipStmt;

typedef struct {
    ErfTag tag;
    union {
        u8     boolean;
        i32    int32;
        i64    int64;
        u64    uint64;
        f64    float64;
        ErfStr str;
        ErfArr arr;
        ErfMap map;
    };
} ErfMapAny;

typedef struct
{
  int txCount;
  RipStmt stmt_beginTx;
  RipStmt stmt_endTx;
  RipStmt replaceIntoGuild;
  RipStmt replaceIntoGuildChannel;
  RipStmt replaceIntoGuildVoiceChannel;
  RipStmt replaceIntoGuildChannelCategory;
  RipStmt replaceIntoGuildMember;
  RipStmt replaceIntoGuildMemberRole;
  RipStmt deleteFromGuildMemberRole_UserId_GuildId;
  RipStmt deleteFromGuildMember_UserId_GuildId;
  RipStmt replaceIntoGuildBan_GuildId_UserId;
  RipStmt deleteFromGuildBan_GuildId_UserId;
  RipStmt replaceIntoRole;
  RipStmt deleteFromRole;
  RipStmt deleteGuildChannelPermQ;
  RipStmt replaceIntoGuildChannelPermQ;
  RipStmt replaceIntoMessage;
  RipStmt insertOrIgnoreIntoQuotedMessage;
  RipStmt replaceIntoEmoji;
  RipStmt replaceIntoEmojiRole;
  RipStmt replaceIntoEmojiGuild;
  RipStmt deleteEmojiInGuildQ;
  RipStmt ensureUserExistsQ;
  RipStmt deleteMessageEmbedsQ;
  RipStmt replaceIntoUserChannelReadState;
  RipStmt replaceIntoUserChannelReadStateKeepMentions;
  RipStmt replaceIntoAnyChannelType;
  RipStmt updateUserStatus;
  RipStmt updateUserStatusWithGame;
  RipStmt insertOrReplaceIntoVoiceState;
  RipStmt deleteFromVoiceState_SessionId;
  RipStmt selectUserHashSum;
  RipStmt replaceIntoUser;
  RipStmt updateUser;
  RipStmt replaceIntoGuildMemberForChannel;
  RipStmt updatePrivateChannelLastMessageId;
  RipStmt updateGuildChannelLastMessageId;
  RipStmt addEmojiReactionQ;
  RipStmt addEmojiReactionCountQ;
  RipStmt removeEmojiReactionQ;
  RipStmt removeEmojiReactionCountQ;
  RipStmt deleteFromMessageEmojiReaction;
  RipStmt deleteFromMessageEmojiReactionCount;
  RipStmt replaceIntoChannelPinnedMessage;
  RipStmt deleteFromChannelPinnedMessage;
  RipStmt selectAuthorFromMessage;
  RipStmt replaceIntoPrivateChan_ChanId;
  RipStmt replaceIntoPrivateChanParticipant_ChanId_UserId;
  RipStmt updatePrivChan_Name_OwnerId_Icon_Id;
  RipStmt insertOrReplaceIntoUserRelationship_OwnUserId_OtherUserId_Type;
  RipStmt clearImageAttachsQ;
  RipStmt clearFileAttachsQ;
  RipStmt addImageAttachQ;
  RipStmt addFileAttachQ;
  RipStmt insertImageEmbedQ;
  RipStmt insertRichEmbedQ;
  RipStmt updateMessageSetDeleted_MsgId_ChanId;
  RipStmt replaceIntoFetchMarker;
  RipStmt selectMsgIdFromFetchMarker;
  void *db;
  void *jsbuffer;
} DisDbPrepared;
