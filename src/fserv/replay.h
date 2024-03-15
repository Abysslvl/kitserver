// replay.h

#ifndef _REPLAY_H
#define _REPLAY_H

#pragma pack(push,1)

typedef struct _REPLAY_PLAYER_INFO
{
    WORD unknown1;
    BYTE unknown2;
    BYTE specialHair;
    DWORD faceHairBits;
    BYTE unknown3[0x2e];
    BYTE specialFace;
    BYTE unknown4[7];
    WORD index;
    BYTE unknown5[3];
    char name[0x2e];
    char nameOnShirt[0x10];
    BYTE unknown6[3];
} REPLAY_PLAYER_INFO;

typedef struct _REPLAY_DATA_PAYLOAD
{
    BYTE miscInfo[0xec];
    REPLAY_PLAYER_INFO players[22];
} REPLAY_DATA_PAYLOAD;

#pragma pack(pop)

#endif
