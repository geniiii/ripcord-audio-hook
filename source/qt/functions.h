#define WRITE_DATAGRAM(name) i64 name(void* this, u8* data, i64 size, void* addr, u16 port)
typedef WRITE_DATAGRAM(WriteDatagramType);

static WriteDatagramType*   write_datagram;
