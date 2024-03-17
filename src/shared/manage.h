// manage.h
#ifndef _MANAGE_
#define _MANAGE_

typedef struct _KMOD {
        DWORD id;
        wchar_t nameLong[BUFLEN];
        wchar_t nameShort[BUFLEN];
        DWORD debug;
} KMOD;

typedef struct _PESINFO {
        wchar_t myDir[BUFLEN];
        wchar_t processFile[BUFLEN];
        wchar_t shortProcessFile[BUFLEN];
        wchar_t shortProcessFileNoExt[BUFLEN];
        wchar_t pesDir[BUFLEN];
        wchar_t gdbDir[BUFLEN];
        wchar_t logName[BUFLEN];
        int gameVersion;
        int realGameVersion;
        wchar_t lang[32];
        HANDLE hProc;
        UINT bbWidth;
        UINT bbHeight;
        double stretchX;
        double stretchY;
} PESINFO;

enum {
    gvPES2013demo,   // PES2013 demo
};

#endif
