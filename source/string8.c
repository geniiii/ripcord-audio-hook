typedef struct {
    union {
        u8*   s;
        u8*   str;
        char* cstr;
        void* data;
    };
    u64 size;
} String8;

#define S8Lit(s) (String8) S8LitComp(s)
#define S8LitComp(s) \
    { (u8*) (s), sizeof(s) - 1 }
