// BAL (Be-A-Legend)
// save file structure and utils

#pragma pack(push, 1)

typedef struct _BAL {
    union {
        struct {
            BYTE unknown1[0x156e94];
            PLAYER_DETAILS playerDetails;
        } bal1;
        struct {
            BYTE unknown1[0x781598];
            PLAYER_INFO player;
        } bal2;
        struct {
            BYTE unknown1[0x781598];
            BYTE unknown2[4+0x2c];
            BYTE faceBit;
            BYTE hairBit;
        } bal4;
        struct {
            BYTE unknown1[0x156e4c];
            BYTE faceBit;
            BYTE hairBit;
        } bal5;
    };
} BAL;

#pragma pack(pop)

