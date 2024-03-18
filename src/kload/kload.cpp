/* KitServer Loader */
#define UNICODE
#define THISMOD &k_kload
#define _COMPILING_KLOAD

#include <windows.h>
#include "shared.h"
#include "manage.h"
#include "log.h"
#include "configs.h"
#include "imageutil.h"
#include "detect.h"
#include "kload.h"
#include "kload_addr.h"
#include "dllinit.h"
#include "lang.h"
#include "hooklib.h"
#include "hook.h"
#define lang(s) getTransl("kload",s)

// GLOBALS
CRITICAL_SECTION g_cs;
bool _noshade = false;

// VARIABLES
HINSTANCE hInst = NULL;
KMOD k_kload = {MODID, NAMELONG, NAMESHORT, DEFAULT_DEBUG};
PESINFO g_pesinfo = {
	NULLSTRING,				// myDir
	NULLSTRING,				// processFile
	NULLSTRING,				// shortProcessFile
	NULLSTRING,				// shortProcessFileNoExt
	NULLSTRING,				// pesDir
	DEFAULT_GDB_DIR,	// gdbDir
	NULLSTRING,				// logName
	-1,								// gameVersion
	-1,								// realGameVersion
	L"eng",						// lang
	INVALID_HANDLE_VALUE,	// hProc
	1024,							// bbWidth
	768,							// bbHeight
	1.0,							// stretchX
	1.0,							// bbWidth
};
extern wchar_t* GAME[];

static hook_manager _hook_manager;
DWORD lastCallSite = 0;

// FUNCTIONS
void setPesInfo();
void kloadLoadDlls(char* pName, const wchar_t* pValue, DWORD a);
void kloadConfig(char* pName, const void* pValue, DWORD a);
KEXPORT DWORD GetLoaderVersion();

/**
 * Other modules can call this to determine whether it is 
 * save to proceed with loading/initialization tasks.
 */
KEXPORT DWORD GetLoaderVersion()
{
    return MAKELONG(KITSERVER_VERSION_MINOR, KITSERVER_VERSION_MAJOR); // 12.2.x
}

/*******************/
/* DLL Entry Point */
/*******************/
EXTERN_C BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		hInst = hInstance;
		
		setPesInfo();
        //SYSTEMTIME st;
        //GetLocalTime(&st);

        wchar_t logName[1024];
        memset(logName, 0, sizeof(logName));
        swprintf(logName, L"%s.%d", //.%03d", 
            g_pesinfo.logName, GetCurrentProcessId());//, st.wMilliseconds);

		//OpenLog(logName);//g_pesinfo.logName);
		OpenLog(g_pesinfo.logName);
		LOG(L"Log started.");
		RegisterKModule(THISMOD);

        /*
        // restore entry point (to avoid live_pe_crc error)
        if (g_pesinfo.gameVersion == gvPES2012)
        {
            DWORD newProtection = PAGE_EXECUTE_READWRITE;
            DWORD g_savedProtection;
            if (VirtualProtect((BYTE*)0x400d48, 4, 
                        newProtection, &g_savedProtection))
            {
                *(DWORD*)0x400d48 = ????;//TODO
                TRACE(L"Entry point restored.");
            }
        }
        else if (g_pesinfo.gameVersion == gvPES2012v101)
        {
            DWORD newProtection = PAGE_EXECUTE_READWRITE;
            DWORD g_savedProtection;
            if (VirtualProtect((BYTE*)0x400d48, 4, 
                        newProtection, &g_savedProtection))
            {
                *(DWORD*)0x400d48 = 0x022f5b60;
                LOG(L"Entry point restored.");
            }

            if (VirtualProtect((BYTE*)0x26f9fe0, 0x20,
                newProtection, &g_savedProtection)) {
                memset((BYTE*)0x26f9fe0, 0, 0x20);
                LOG(L"restored 1");
            }
            newProtection = PAGE_READWRITE;
            if (VirtualProtect((BYTE*)0x17171e0, 0x20,
                newProtection, &g_savedProtection)) {
                memset((BYTE*)0x17171e0, 0, 0x20);
                LOG(L"restored 2");
            }
        }
        */

        // initialize global critical section
        InitializeCriticalSection(&g_cs);
		
        if (g_pesinfo.realGameVersion != -1)
        {
            LOG(L"Game version: %s", GAME[g_pesinfo.realGameVersion]);
        }
		if (!checkGameVersion()) 
        {
			LOG(L"Sorry, your game version isn't supported!");
			return false;
		}
		
		_hook_manager.SetCallHandler(MasterCallFirst);
		
		// read configuration
        wstring cfg_file(g_pesinfo.myDir);
        cfg_file += L"config.txt";
        if (!readConfig(cfg_file.c_str())) {
            // try default config
            LOG(L"Couldn't open config.txt. Trying config-default.txt");
            cfg_file = g_pesinfo.myDir;
            cfg_file += L"config-default.txt";
            if (!readConfig(cfg_file.c_str())) {
                LOG(L"Couldn't open config-default.txt!");
            }
        }

        wchar_t currDir[BUFLEN];
        GetCurrentDirectory(BUFLEN, currDir);
        LOG(L"CurrentDirectory: %s", currDir);
        SetCurrentDirectory(g_pesinfo.myDir);

		_getConfig("kload", "gdb.dir", DT_STRING, 1, kloadConfig);
		_getConfig("kload", "debug", DT_DWORD, 2, kloadConfig);
		_getConfig("kload", "lang", DT_STRING, 3, kloadConfig);
        _getConfig("kload", "noshade", DT_DWORD, 4, kloadConfig);
		_getConfig("kload", "dll", DT_STRING, C_ALL, (PROCESSCONFIG)kloadLoadDlls);
        SetCurrentDirectory(currDir);
		
		// adjust gdbDir, if it is specified as relative path
		if (g_pesinfo.gdbDir[0] == '.')
		{
			// it's a relative path. Therefore do it relative to myDir
			wchar_t temp[BUFLEN];
			ZeroMemory(temp, WBUFLEN);
			wcscpy(temp, g_pesinfo.myDir); 
			wcscat(temp, g_pesinfo.gdbDir);
			
			ZeroMemory(g_pesinfo.gdbDir, WBUFLEN);
			wcscpy(g_pesinfo.gdbDir, temp);
		}
		
		wchar_t langFile[BUFLEN];
		ZeroMemory(langFile, WBUFLEN);
		swprintf(langFile, L"%s.\\lang_%s.txt", g_pesinfo.myDir, g_pesinfo.lang);
		readLangFile(langFile, hInstance);
		
		/*#ifdef MYDLL_RELEASE_BUILD
		//if debugging shouldn't have been enabled, delete log file
		if (k_kload.debug < 1) {
			CloseLog();
			DeleteFile(g_pesinfo.logName);
		}
		#endif*/
		
		initAddresses();

        hookDirect3DCreate9();
	}
	
	else if (dwReason == DLL_PROCESS_DETACH)
	{
        //__asm int 3;

		LOG(L"Closing log.");
		CloseLog();

        // dispose of critical section
        DeleteCriticalSection(&g_cs);
	}
	
	return true;
}



KEXPORT PESINFO* getPesInfo()
{
	return &g_pesinfo;
}

void setPesInfo()
{	
	// determine my directory (myDir)
	ZeroMemory(g_pesinfo.myDir, WBUFLEN);
	GetModuleFileName(hInst, g_pesinfo.myDir, BUFLEN);
	wchar_t *q = g_pesinfo.myDir + wcslen(g_pesinfo.myDir);
	while ((q != g_pesinfo.myDir) && (*q != '\\')) { *q = '\0'; q--; }
	
	// hProc
	g_pesinfo.hProc = GetModuleHandle(NULL);
	
	// processFile
	ZeroMemory(g_pesinfo.processFile, WBUFLEN);
	GetModuleFileName(NULL, g_pesinfo.processFile, BUFLEN);
	
	// pesDir
	ZeroMemory(g_pesinfo.pesDir, WBUFLEN);
	wcscpy(g_pesinfo.pesDir, g_pesinfo.processFile);
	wchar_t *q1 = g_pesinfo.pesDir + wcslen(g_pesinfo.pesDir);
	while ((q1 != g_pesinfo.pesDir) && (*q1 != '\\')) { *q1 = '\0'; q1--; }
	
	// shortProcessFile
	ZeroMemory(g_pesinfo.shortProcessFile, WBUFLEN);
	wchar_t* zero = g_pesinfo.processFile + wcslen(g_pesinfo.processFile);
	wchar_t* p = zero; while ((p != g_pesinfo.processFile) && (*p != '\\')) p--;
	if (*p == '\\') p++;
	wcscpy(g_pesinfo.shortProcessFile, p);
	
	// save short filename without ".exe" extension (shortProcessFileNoExt)
	ZeroMemory(g_pesinfo.shortProcessFileNoExt, WBUFLEN);
    /*
	wchar_t* ext = g_pesinfo.shortProcessFile + wcslen(g_pesinfo.shortProcessFile) - 4;
	if (wcsicmp(ext, L".exe")==0) {
		memcpy(g_pesinfo.shortProcessFileNoExt, g_pesinfo.shortProcessFile, SW * (ext - g_pesinfo.shortProcessFile)); 
	}
	else {
		wcscpy(g_pesinfo.shortProcessFileNoExt, g_pesinfo.shortProcessFile);
	}
    */
    wcscpy(g_pesinfo.shortProcessFileNoExt, g_pesinfo.shortProcessFile);
    wchar_t* end = wcsrchr(g_pesinfo.shortProcessFileNoExt, '.');
    if (end) {
        end[0] = '\0';
    }
	
	// logName
	ZeroMemory(g_pesinfo.logName, WBUFLEN);
	wcscpy(g_pesinfo.logName, g_pesinfo.myDir);
	wcscat(g_pesinfo.logName, L"log\\");
	wcscat(g_pesinfo.logName, g_pesinfo.shortProcessFileNoExt); 
	wcscat(g_pesinfo.logName, L".log");

	g_pesinfo.realGameVersion = GetRealGameVersion();
	g_pesinfo.gameVersion = GetGameVersion(g_pesinfo.realGameVersion);

	return;
}

KEXPORT void RegisterKModule(KMOD *k)
{
	LOG(L"Registering module %s (\"%s\")", k->nameShort, k->nameLong);
	return;
}

KEXPORT void getConfig(char* section, char* name, BYTE dataType, DWORD a, PROCESSCONFIG callback) {
	_getConfig(section, name, dataType, a, callback);
	return;
}

KEXPORT const wchar_t* getTransl(char* section, char* key) {
	return _getTransl(section, key);
}

void kloadLoadDlls(char* pName, const wchar_t* pValue, DWORD a)
{	
	wchar_t dllName[BUFLEN];
	ZeroMemory(dllName, WBUFLEN);

	wcscpy(dllName, pValue);
	// check for C:, D: etc and things like %windir%
	if (dllName[1] != ':' && dllName[0] != '%')
		{
			// it's a relative path. Therefore do it relative to myDir
			wchar_t temp[BUFLEN];
			ZeroMemory(temp, WBUFLEN);
			wcscpy(temp, g_pesinfo.myDir); 
			wcscat(temp, dllName);
			
			ZeroMemory(dllName, WBUFLEN);
			wcscpy(dllName, temp);
		}
	
	LOG(L"Loading module \"%s\" ...", dllName);
	if (LoadLibrary(dllName) == NULL)
		LOG(L"... ERROR!");
	else
		LOG(L"... OK!");
	 
	return;
}

void kloadConfig(char* pName, const void* pValue, DWORD a)
{
	switch (a) {
		case 1:	//gdb.dir
			wcscpy(g_pesinfo.gdbDir, (wchar_t*)pValue);
            // ensure trailing slash
            if (wcslen(g_pesinfo.gdbDir)>0)
            {
                if (g_pesinfo.gdbDir[wcslen(g_pesinfo.gdbDir)-1]!='\\')
                    wcscat(g_pesinfo.gdbDir, L"\\");
            }
			break;
		case 2: // debug
			k_kload.debug = *(DWORD*)pValue;
			break;
		case 3:	// lang
			wcscpy(g_pesinfo.lang, (wchar_t*)pValue);
			break;
        case 4: // noshade
            _noshade = *(DWORD*)pValue == 1;
            break;
	}
	return;
}


// hooking

KEXPORT bool MasterHookFunction(DWORD call_site, DWORD numArgs, void* target)
{
    hook_point hp(call_site, numArgs, (DWORD)target);
    return _hook_manager.hook(hp);
}

KEXPORT bool MasterUnhookFunction(DWORD call_site, void* target)
{
    hook_point hp(call_site, 0, (DWORD)target);
    return _hook_manager.unhook(hp);
}

DWORD oldEBP1;

DWORD MasterCallFirst(...)
{
	DWORD oldEAX, oldEBX, oldECX, oldEDX;
	DWORD oldEBP, oldEDI, oldESI;
	int i;
	__asm {
		mov oldEAX, eax
		mov oldEBX, ebx
		mov oldECX, ecx
		mov oldEDX, edx
		mov oldEBP, ebp
		mov oldEBP1, ebp
		mov oldEDI, edi
		mov oldESI, esi
	};
	
	DWORD arg;
	DWORD result=0;
	bool lastAddress=false;
	DWORD jmpDest=0;

	DWORD call_site=*(DWORD*)(oldEBP+4)-5;
	DWORD addr=_hook_manager.getFirstTarget(call_site, &lastAddress);
	
	if (addr==0) return;
	
	DWORD before=lastCallSite;
	lastCallSite=call_site;
	
	DWORD numArgs=_hook_manager.getNumArgs(call_site);
	bool wasJump=_hook_manager.getType(call_site)==HOOKTYPE_JMP;
	
	if (wasJump && lastAddress) {
		result=oldEAX;
		goto EndOfCallF;
	};
	
	//writing this as inline assembler allows to
	//give as much parameters as we want and more
	//important, we can restore all registers
	__asm {
		//push ebp
	};
	
	for (i=numArgs-1;i>=0;i--) {
		if (wasJump)
			arg=*((DWORD*)oldEBP+3+i);
		else
			arg=*((DWORD*)oldEBP+2+i);
		__asm mov eax, arg
		__asm push eax
	};
	
	__asm {
		//restore registers
		mov eax, oldEAX
		mov ebx, oldEBX
		mov ecx, oldECX
		mov edx, oldEDX
		mov edi, oldEDI
		mov esi, oldESI
		//mov ebp, oldEBP
		//mov ebp, [ebp]
		
		call ds:[addr]
		
		mov result, eax
		
	};
	
	for (i=0;i<numArgs;i++)
		__asm pop eax
	
	__asm {
		//pop ebp
		mov eax, result
	};
	
	EndOfCallF:
	
	lastCallSite=before;
	
	if (wasJump) {
		if (lastAddress)
			jmpDest=addr;
		else
			jmpDest=_hook_manager.getOriginalTarget(call_site);
		
		//change the return address to the destination of our jump
		*(DWORD*)(oldEBP+4)=jmpDest;
	}
	
	return result;
};

KEXPORT DWORD MasterCallNext(...)
{
	if (lastCallSite==0) return 0;
	
	DWORD oldEAX, oldEBX, oldECX, oldEDX;
	DWORD oldEBP, oldEDI, oldESI, numArgs;
	int i;
	__asm {
		mov oldEAX, eax
		mov oldEBX, ebx
		mov oldECX, ecx
		mov oldEDX, edx
		mov oldEBP, ebp
		mov oldEDI, edi
		mov oldESI, esi
	};
	
	DWORD result=0;
	DWORD arg;
	bool lastAddress=false;
	
	DWORD addr=_hook_manager.getNextTarget(lastCallSite, &lastAddress);
	bool wasJump=_hook_manager.getType(lastCallSite)==HOOKTYPE_JMP;
	if (addr==0) return 0;
	
	//Don't call a jump's last address (its original destination)
	if (wasJump && lastAddress) {
		result=oldEAX;
		//restore registers
		__asm {
			mov eax, oldEAX
			mov ebx, oldEBX
			mov ecx, oldECX
			mov edx, oldEDX
			mov edi, oldEDI
			mov esi, oldESI
		};
		goto EndOfCallN;
	};
	
	numArgs=_hook_manager.getNumArgs(lastCallSite);

	__asm {
		//push ebp
	};
	
	for (i=numArgs-1;i>=0;i--) {
		arg=*((DWORD*)oldEBP+2+i);
		__asm mov eax, arg
		__asm push eax
	};
	
	__asm {
		//restore registers
		mov eax, oldEAX
		mov ebx, oldEBX
		mov ecx, oldECX
		mov edx, oldEDX
		mov edi, oldEDI
		mov esi, oldESI
		//mov ebp, oldEBP
		//mov ebp, [ebp]
		
		call ds:[addr]
		
		mov result, eax
	};
	
	for (i=0;i<numArgs;i++)
		__asm pop eax
	
	__asm mov eax, result
	
	EndOfCallN:
	
	return result;
}

KEXPORT int DumpData(void* data, size_t size)
{
    static int count = 0;
    wchar_t filename[1024] = {0};
    swprintf(filename, L"%s\\dump%03d.bin", 
            getPesInfo()->myDir, count);
    FILE* f = _wfopen(filename,L"wb");
    if (f) 
    {
        fwrite(data, size, 1, f);
        fclose(f);
        LOG(L"Dumped file (count=%d)",count);
    }
    else
    {
        LOG(L"ERROR: unable to dump file (count=%d). Error code = %d",count, errno);
    }
    count++;
    return count-1;
}

