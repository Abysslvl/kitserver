// kserv.h
#include <kconf.h>

#define MODID 100
#define NAMELONG L"Kit Server " KITSERVER_VERSION_STRING
#define NAMESHORT L"KSERV"
#define DEFAULT_DEBUG 0

#pragma pack(4)

typedef DWORD (*INIT_NEW_KIT)(DWORD);

KEXPORT DWORD hookVtableFunction(DWORD vtableAddr, DWORD offset, void* func);

////////////////////////////////////////////////////

typedef struct _PES_TEXTURE {
    DWORD* vtable;	// == 0xffab80 for PES2009
	BYTE unknown1[8];
	DWORD width;
	DWORD height;
	BYTE unknown2[0xc];
	IDirect3DTexture9* tex;
} PES_TEXTURE;

typedef struct _PES_TEXTURE_PACKAGE {
    DWORD* vtable;	// == 0x1009df8 for PES2009
	BYTE unknown1[8];
	PES_TEXTURE* tex;
} PES_TEXTURE_PACKAGE;

/*
typedef struct _KIT_OBJECT {
    DWORD* vtable;	// == code[C_KIT_VTABLE]
	BYTE unknown1[0x40];
	PES_TEXTURE_PACKAGE* kitTex; 
	PES_TEXTURE_PACKAGE* flagTex; 
	BYTE unknown2[0x30];
	WORD teamId;
	BYTE unknown3[6];
	BYTE isAfsKit;
    BYTE unknown4[3];
	DWORD kitIdx;
} KIT_OBJECT;
*/

typedef struct _KIT_OBJECT {
    DWORD* vtable;	// == code[C_KIT_VTABLE]
	BYTE unknown1[0x44];
	PES_TEXTURE_PACKAGE* kitTex; 
	PES_TEXTURE_PACKAGE* flagTex; 
	PES_TEXTURE_PACKAGE* otherTextures[8];
	BYTE isAfsKit;
    BYTE unknown2[3];
    WORD model;
    BYTE attributes[0x12];
    WORD teamId;
    WORD unknown3;
    DWORD unknown4;
} KIT_OBJECT;

////////////////////////////////////////////////////

typedef struct _TEAM_NAME {
    char name[0x48];
    char name01[0x48];
    char name02[0x48];
    char name03[0x48];
    char name04[0x48];
    char name05[0x48];
    char name06[0x48];
    char name07[0x48];
    char name08[0x48];
    char name09[0x48];
    char name10[0x48];
    char name11[0x48];
    char name12[0x48];
    char name13[0x48];
    char name14[0x48];
    char name15[0x48];
    char name16[0x48];
    char name17[0x48];
    char name18[0x48];
    char name19[0x48];
    char abbr[4];
    WORD teamId;
    WORD teamId2;
    BYTE unknown2[8];
    DWORD id;
} TEAM_NAME; 

typedef struct _PNG_CHUNK_HEADER {
    DWORD dwSize;
    DWORD dwName;
} PNG_CHUNK_HEADER;

typedef struct _TEXTURE_ENTRY_HEADER
{
    BYTE sig[4]; // "WE00"
    BYTE unknown1; // 0
    BYTE unknown2; // 0
    BYTE unknown3; // 03
    BYTE bpp;
    WORD width;
    WORD height;
    WORD paletteOffset;
    WORD dataOffset;
} TEXTURE_ENTRY_HEADER;

typedef struct _PALETTE_ENTRY
{
    BYTE b;
    BYTE g;
    BYTE r;
    BYTE a;
} PALETTE_ENTRY;

typedef struct _TEXTURE_ENTRY
{
    TEXTURE_ENTRY_HEADER header;
    PALETTE_ENTRY palette[256];
    BYTE data[1];
} TEXTURE_ENTRY;

////

typedef struct _KIT_CHOICE
{
    BYTE other1[0x70];
    BYTE homeAway;
    BYTE other2[3];
    BYTE other3[0x68];
    WORD model;
    //BYTE attributes[0x12];
    BYTE a00;
    BYTE numberY;
    BYTE numberX;
    BYTE numberSize;
    BYTE frontNumberY;
    BYTE frontNumberX;
    BYTE frontNumberSize;
    BYTE shortsNumberY;
    BYTE shortsNumberX;
    BYTE shortsNumberSize;
    BYTE shortsNumberLocation;
    BYTE frontNumberShow;
    BYTE a0c;
    BYTE a0d;
    BYTE a0e;
    BYTE a0f;
    BYTE a10;
    BYTE a11;
    WORD teamId;
    WORD other4;
    BYTE other5[4];
    BYTE isEditedKit;
    BYTE other51[3];
    WORD kitSlotIndex;
    WORD kitAfsFlag;
    BYTE other6[0x28];
    WORD fontNumSlotIndex;
    WORD fontNumAfsFlag;
    BYTE other7[0x10];
    BYTE uniformSelection;
    BYTE uniformSelectionOtherBytes[2];
    BYTE other8;
    BYTE other9[2];
    KCOLOR shortsColor;
    BYTE other10[0x18];
    BYTE other11[2];
    BYTE sleevePatchRightPosShort;
    BYTE sleevePatchRightPosLong;
    BYTE sleevePatchLeftPosShort;
    BYTE sleevePatchLeftPosLong;
    BYTE other12[2];
    BYTE other13[0x154];
    BYTE other14[2];
    BYTE other15[0x20];
    BYTE reloadFlag;
    BYTE other16;

} KIT_CHOICE;

typedef struct _EURO_TEAM_STRUCT {
    DWORD unknown1[0xB];
    TEAM_KIT_INFO* begin;
    TEAM_KIT_INFO* end;
    TEAM_KIT_INFO* end_again;

} EURO_TEAM_STRUCT;

