typedef struct QJsonPrivate_Data  QJsonPrivate_Data;
typedef struct QJsonPrivate_Array QJsonPrivate_Array;

typedef struct
{
    QJsonPrivate_Data*  d;
    QJsonPrivate_Array* a;
} QJsonArray;

typedef enum {
    QJsonValue_Type_Null,
    QJsonValue_Type_Bool,
    QJsonValue_Type_Double,
    QJsonValue_Type_String,
    QJsonValue_Type_Array,
    QJsonValue_Type_Object,
    QJsonValue_Type_Undefined = 0x80,
} QJsonValue_Type;

typedef struct __declspec(align(8)) {
    u64                u0;
    QJsonPrivate_Data* d;
    QJsonValue_Type    t;
} QJsonValue;
