/**
	These functions are used by nearly all dlls. The following variables/
	defines are necessary:
	
	---
	
	BYTE allowedGames[] = {
		gvPES2008demo,
		gvPES2008demoSet,
	};
	
	#define CODELEN xxx
	DWORD codeArray[][CODELEN] = {
		// ABC
		{
			1, 2, 3,
		},
		// DEF
		{
			4, 5, 6,
		},
	};
	
	#define DATALEN yyy
	DWORD dataArray[][DATALEN] = {
		...
	};

	DWORD code[CODELEN];
	DWORD data[DATALEN];
	//int gameVersion;		// only for dlls which don't have access to getPesInfo()
	
	---
	
	Setting CODELEN (DATALEN) to 0 and/or using
	DWORD* codeArray(dataArray) = NULL;
	is allowed, if no code (data) addresses are needed. However, the variables
	have to be defined even in that case.
	
	---
	
	Needed includes:
	#include "MODULE_addr.h"
	#include "dllinit.h"
	
	dllinit.h has to be defined AFTER all of these headers!
	
*/

inline bool checkGameVersion()
{
	#ifdef GETPESINFO
	int gameVersion = getPesInfo()->gameVersion;
	#else // #ifdef GETPESINFO
	gameVersion = GetGameVersion();
	#endif // #ifdef GETPESINFO
	
	// check if this is in the array of allowed games
	bool inAllowedGames = false;
	for (int i=0; i<sizeof(allowedGames); i++)
		if (allowedGames[i] == gameVersion) {inAllowedGames = true; break;}
	if (!inAllowedGames) return false;

	// looks good, we can start
	return true;
}

#ifndef _COMPILING_KLOAD
inline void copyAdresses()
{
	#ifdef GETPESINFO
	int gameVersion = getPesInfo()->gameVersion;
	#endif // #ifdef GETPESINFO

	if (codeArray)
		memcpy(code, codeArray[gameVersion], sizeof(code));
	else
		ZeroMemory(code, sizeof(code));
		
	if (dataArray)
		memcpy(dta, dataArray[gameVersion], sizeof(dta));
	else
		ZeroMemory(dta, sizeof(dta));

	return;
}

#define CHECK_KLOAD(minVer) {\
    FARPROC GetLoaderVersion = GetProcAddress(\
            GetModuleHandle(L"kload"),\
            "GetLoaderVersion");\
    if (!GetLoaderVersion || GetLoaderVersion() < minVer)\
    {\
        LOG(L"ERROR: This module requires kload.dll %d.%d.0 or newer.",\
                HIWORD(minVer), LOWORD(minVer));\
        return false;\
    }\
}

#endif //_COMPILING_KLOAD
