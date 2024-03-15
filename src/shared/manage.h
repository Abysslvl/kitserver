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
    gvPES2011demo,   // PES2011 PC DEMO
    gvPES2011,       // PES2011 PC 1.0 
    gvPES2011v101,   // PES2011 PC 1.01
    gvPES2011v102,   // PES2011 PC 1.02
    gvPES2011v103,   // PES2011 PC 1.03
    gvPES2012demo,   // PES2012 PC DEMO
    gvPES2012demo2,  // PES2012 PC DEMO2
    gvPES2012,       // PES2012 PC
    gvPES2012v101,   // PES2012 PC 1.01
    gvPES2012v102,   // PES2012 PC 1.02
    gvPES2012v103,   // PES2012 PC 1.03
    gvPES2012v106,   // PES2012 PC 1.06
};

#endif
