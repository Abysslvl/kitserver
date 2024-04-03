/* KitServer Loader Header-File */
#ifndef _KLOAD_H
#define _KLOAD_H

#include <kconf.h>

#ifdef _COMPILING_KLOAD
#define MODID 0
#define NAMELONG L"Kitserver Loader " KITSERVER_VERSION_STRING
#define NAMESHORT L"KLOAD"

#define DEFAULT_DEBUG 0
#define DEFAULT_GDB_DIR L".\\"
#endif // _COMPILING_KLOAD

#define KEXPORT EXTERN_C __declspec(dllexport)

#ifndef _COMPILING_KLOAD
typedef void  (*PROCESSCONFIG)(char* pName, const void* pValue, DWORD a);
enum {DT_NORMAL, DT_STRING, DT_DWORD, DT_INT, DT_DOUBLE, DT_FLOAT};
#define C_ALL 0x80000000
#endif // _COMPILING_KLOAD

// exported functions
KEXPORT PESINFO* getPesInfo();
#define GETPESINFO
KEXPORT void RegisterKModule(KMOD *k);
KEXPORT void getConfig(const char* section, char* name, BYTE dataType, DWORD a, PROCESSCONFIG callback);
KEXPORT const wchar_t* getTransl(char* section, char* key);

KEXPORT bool MasterHookFunction(DWORD call_site, DWORD numArgs, void* target);
KEXPORT bool MasterUnhookFunction(DWORD call_site, void* target);
DWORD MasterCallFirst(...);
KEXPORT DWORD MasterCallNext(...);
KEXPORT int DumpData(void* data, size_t size);

// global critical section
extern CRITICAL_SECTION g_cs;

#endif
