#ifndef _JUCE_AFSREADER_
#define _JUCE_AFSREADER_

#include <stdio.h>

// AFS folder signature
#define AFSSIG 0x00534641

// Error codes for function return values
#define AFS_OK 0
#define AFS_FOPEN_FAILED -1
#define AFS_HEAPALLOC_FAILED -2
#define AFS_ITEM_NOT_FOUND -3

typedef struct _AFSDIRHEADER {
	DWORD dwSig; // 4-byte signature: "AFS\0"
	DWORD dwNumFiles; // number of files in the afs
} AFSDIRHEADER;

typedef struct _AFSITEMINFO {
	DWORD dwOffset; // offset from the beginning of afs
	DWORD dwSize; // size of the item
} AFSITEMINFO;

typedef struct _AFSNAMEINFO {
	char szFileName[32]; // Zero-terminated filename string
	BYTE other[16]; // misc info
} AFSNAMEINFO;

char* GetAfsErrorText(DWORD errorCode);
DWORD GetItemInfo(char* afsFileName, char* uniFileName, AFSITEMINFO* itemInfo, DWORD* base);
DWORD GetItemInfoById(char* afsFileName, int id, AFSITEMINFO* itemInfo, DWORD* base);
DWORD ReadItemInfoById(FILE* f, DWORD id, AFSITEMINFO* itemInfo, DWORD base);
DWORD ReadItemInfoById(HANDLE hfile, DWORD id, AFSITEMINFO* itemInfo, DWORD base);

typedef struct _PACKED_BIN_HEADER
{
    BYTE sig[8]; // \0x0\x01\x01WESYS
    DWORD sizePacked;
    DWORD sizeUnpacked;
} PACKED_BIN_HEADER;

typedef struct _PACKED_BIN
{
    PACKED_BIN_HEADER header;
    BYTE data[1];
} PACKED_BIN;

typedef struct _UNPACKED_BIN_HEADER
{
    DWORD numEntries;
    DWORD unknown1; // often has value of 8
} UNPACKED_BIN_HEADER;

typedef struct _ENTRY_INFO 
{
    DWORD offset;
    DWORD size;
    DWORD indexOffset; // (if index doesn't exist, then this can be 0 or 0xffffffff)
} ENTRY_INFO; 

typedef struct _UNPACKED_BIN
{
    UNPACKED_BIN_HEADER header;
    ENTRY_INFO entryInfo[1];
} UNPACKED_BIN;

#endif
