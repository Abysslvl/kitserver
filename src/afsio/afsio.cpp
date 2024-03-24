/* <afsio>
 *
 * This module is the foundation for any AFS-replacement activities.
 * It provides hooks to various points in time, where BINs are being
 * read from the AFS. 
 *
 * (For example, AFS2FS module is a client of this module.)
 */
#define UNICODE
#define THISMOD &k_afsio

#include <kconf.h>

#define MODID 123
#ifdef DEBUG
#define NAMELONG L"AFSIO Module " KITSERVER_VERSION_STRING " (DEBUG)"
#else
#define NAMELONG L"AFSIO Module " KITSERVER_VERSION_STRING
#endif
#define NAMESHORT L"AFSIO"
#define DEFAULT_DEBUG 0

#include <windows.h>
#include <stdio.h>
#include <sys/stat.h>

#include <map>
#include <list>
#include <hash_map>
#include <wchar.h>

using namespace std;
using namespace stdext;

#include "kload_exp.h"
#include "afsio.h"
#include "afsio_addr.h"
#include "dllinit.h"
#include "pngdib.h"
#include "utf8.h"
#include "names.h"

#define lang(s) getTransl("afsio",s)

#define SWAPBYTES(dw) \
    ((dw<<24 & 0xff000000) | (dw<<8  & 0x00ff0000) | \
    (dw>>8  & 0x0000ff00) | (dw>>24 & 0x000000ff))

#define MAX_RELPATH 0x108
#define MAX_AFSID 31

// VARIABLES
HINSTANCE hInst = NULL;
KMOD k_afsio = {MODID, NAMELONG, NAMESHORT, DEFAULT_DEBUG};

// GLOBALS
hash_map<DWORD,FILE_STRUCT> g_file_map;
hash_map<DWORD,DWORD> g_offset_map;
hash_map<DWORD,FILE_STRUCT> g_event_map;
bool _initialized(false);

static char _imgExt1[] = ".img";
static char _imgExt2[] = ".IMG";
const char* _imgExt1End = _imgExt1+strlen(_imgExt1)-1;
const char* _imgExt2End = _imgExt2+strlen(_imgExt2)-1;
const int _imgExt1Len = strlen(_imgExt1);

// FUNCTIONS
HRESULT STDMETHODCALLTYPE initModule(IDirect3D9* self, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters, 
    IDirect3DDevice9** ppReturnedDeviceInterface);

void afsioAtGetBufferSizeCallPoint();
void afsioAtGetBinSizeCallPoint1();
void afsioAtGetBinSizeCallPoint2();
void afsioAfterGetOffsetPagesCallPoint();
void afsioAfterCreateEventCallPoint();
void afsioAtGetImgSize1CallPoint();
void afsioAtGetImgSize2CallPoint();
void afsioBeforeReadCallPoint();
void afsioAfsReadFileCallPoint();
void afsioAtCloseHandleCallPoint();

KEXPORT DWORD GetAfsIdByBase(DWORD base);
KEXPORT DWORD GetAfsIdByReadEvent(READ_EVENT_STRUCT* res, const char* pathName);
DWORD GetAfsIdByPathName(const char* pathName);
DWORD GetAfsIdByPathNameOld(const char* pathName);
DWORD GetAfsIdByPathNameFast(const char* pathName);
DWORD GetBinSize(DWORD afsId, DWORD binId, DWORD orgSize);
void compareSpeed(const char* pathName);

KEXPORT DWORD afsioAtGetBinSize(DWORD base, DWORD binId, DWORD orgSize);
KEXPORT DWORD afsioAtGetBufferSize(DWORD afsId, DWORD binId, DWORD orgSize);
KEXPORT void afsioAfterGetOffsetPages(
        DWORD offsetPages, DWORD afsId, DWORD binId);
KEXPORT void afsioAfterCreateEvent(
        DWORD eventId, READ_EVENT_STRUCT* res, char* pathName);
KEXPORT void afsioBeforeRead(READ_STRUCT* rs);
KEXPORT void afsioAtCloseHandle(DWORD eventId);
KEXPORT BOOL AfsReadFile(
        HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead,
        LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped,
        FILE_READ_STRUCT* pfrs);

void afsioConfig(char* pName, const void* pValue, DWORD a);

// callbacks chain
static list<CLBK_GET_FILE_INFO> g_afsio_callbacks;


/*******************/
/* DLL Entry Point */
/*******************/
EXTERN_C BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		hInst = hInstance;

		RegisterKModule(THISMOD);
		
		if (!checkGameVersion()) {
			LOG(L"Sorry, your game version isn't supported!");
			return true;
		}

        CHECK_KLOAD(MAKELONG(KITSERVER_VERSION_MINOR, KITSERVER_VERSION_MAJOR));

		copyAdresses();
		hookFunction(hk_D3D_CreateDevice, initModule);
	}
	
	else if (dwReason == DLL_PROCESS_DETACH)
	{
	}
	
	return true;
}

KEXPORT bool afsioAddCallback(const CLBK_GET_FILE_INFO callback)
{
    g_afsio_callbacks.push_back(callback);
    LOG(L"callback registered.");
    return true;
}

KEXPORT bool afsioRemoveCallback(const CLBK_GET_FILE_INFO callback)
{
    LOG(L"removing callbacks is currently unimplemented.");
    return true;
}

void afsioConfig(char* pName, const void* pValue, DWORD a)
{
	switch (a) {
		case 1: // debug
			k_afsio.debug = *(DWORD*)pValue;
			break;
	}
	return;
}

HRESULT STDMETHODCALLTYPE initModule(IDirect3D9* self, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters, 
    IDirect3DDevice9** ppReturnedDeviceInterface) 
{
    if (_initialized) {
        return D3D_OK;
    }

    getConfig("afsio", "debug", DT_DWORD, 1, afsioConfig);
    LOG(L"debug = %d", k_afsio.debug);

	unhookFunction(hk_D3D_CreateDevice, initModule);

    HookCallPoint(code[C_AT_GET_SIZE1], afsioAtGetBinSizeCallPoint1, 6, 2);
    HookCallPoint(code[C_AT_GET_SIZE2], afsioAtGetBinSizeCallPoint2, 6, 2);
    HookCallPoint(code[C_AT_GET_BUFFERSIZE], 
            afsioAtGetBufferSizeCallPoint, 6, 5);
    HookCallPoint(code[C_AFTER_CREATE_EVENT], 
            afsioAfterCreateEventCallPoint, 6, 3);
    HookCallPoint(code[C_AT_GET_IMG_SIZE1], afsioAtGetImgSize1CallPoint, 6, 0);
    HookCallPoint(code[C_AT_GET_IMG_SIZE2], afsioAtGetImgSize2CallPoint, 3, 1);
    HookCallPoint(code[C_AT_CLOSE_HANDLE], afsioAtCloseHandleCallPoint, 6, 3);
    HookCallPoint(code[C_AFTER_GET_OFFSET_PAGES], 
            afsioAfterGetOffsetPagesCallPoint, 6, 1);
    HookCallPoint(code[C_BEFORE_READ], afsioBeforeReadCallPoint, 6, 1);
    HookCallPoint(code[C_BEFORE_READ2], afsioBeforeReadCallPoint, 6, 1);
    
	TRACE(L"Hooking done.");

    //__asm { int 3 }          // uncomment this for debugging as needed
    _initialized = true;
    return D3D_OK;
}

void afsioAtGetBinSizeCallPoint1()
{
    __asm {
        // IMPORTANT: when saving flags, use pusfd/popfd for stack alignment
        pushfd 
        push ebp
        push eax
        push ebx
        push edx
        push esi
        push edi
        mov ecx, dword ptr ds:[esi+edi*4+0x11c] // original code
        push ecx // params: org-size in bytes
        push edi // params: file id
        push esi // params: base
        call afsioAtGetBinSize
        mov ecx, eax
        add esp,0x0c     // pop parameters
        pop edi
        pop esi
        pop edx
        pop ebx
        pop eax
        pop ebp
        popfd
        retn
    }
}

void afsioAtGetBinSizeCallPoint2()
{
    __asm {
        // IMPORTANT: when saving flags, use pusfd/popfd for stack alignment
        pushfd 
        push ebp
        push eax
        push ebx
        push ecx
        push esi
        push edi
        mov edx, dword ptr ds:[esi+edi*4+0x11c] // original code
        push edx // params: org-size in bytes
        push edi // params: file id
        push esi // params: base
        call afsioAtGetBinSize
        add esp,0x0c     // pop parameters
        mov edx, eax
        pop edi
        pop esi
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        retn
    }
}

void afsioAtGetBufferSizeCallPoint()
{
    __asm {
        // IMPORTANT: when saving flags, use pusfd/popfd for stack alignment
        pushfd 
        push ebp
        push ebx
        push ecx
        push edx
        push esi
        push edi
        //mov edx, dword ptr ds:[ebx+0x10]  // original code
        mov eax, dword ptr ds:[ecx+edx*4]
        mov ecx, dword ptr ds:[ebx+0x0c]
        push eax // params: org-size in bytes
        push edx // params: file id
        push ecx // params: afs id
        call afsioAtGetBufferSize
        add esp,0x0c     // pop parameters
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop ebp
        popfd
        xor edx,edx
        retn
    }
}

KEXPORT DWORD afsioAtGetBufferSize(DWORD afsId, DWORD fileId, DWORD orgSize)
{
    TRACE(L"afsioAtGetBufferSize:: afsId=%02x, fileId=%d, orgSize=%08x", 
            afsId, fileId, orgSize);

    return GetBinSize(afsId, fileId, orgSize);
}

KEXPORT DWORD afsioAtGetBinSize(DWORD base, DWORD fileId, DWORD orgSize)
{
    TRACE(L"afsioAtGetBinSize:: base=%p, fileId=%d, orgSize=%08x", 
            base, fileId, orgSize);
    DWORD afsId = GetAfsIdByBase(base);
    if (afsId == 0xffffffff) {
        return orgSize;
    }

    return GetBinSize(afsId, fileId, orgSize);
}

DWORD GetBinSize(DWORD afsId, DWORD fileId, DWORD orgSize)
{
    DWORD result = orgSize;
    HANDLE hfile = NULL;
    DWORD fsize = 0;

    // execute callbacks
    for (list<CLBK_GET_FILE_INFO>::iterator it = g_afsio_callbacks.begin();
            it != g_afsio_callbacks.end(); 
            it++)
    {
        if ((*it)(afsId, fileId, hfile, fsize))
            break;  // first successful callback stops chain processing
    }

    if (hfile && fsize)
    {
        DWORD binKey = (afsId << 16) + fileId;

        // make new entry
        FILE_STRUCT fs;
        fs.hfile = hfile;
        fs.fsize = fsize;
        fs.offset = 0;
        fs.binKey = binKey;

        pair<hash_map<DWORD,FILE_STRUCT>::iterator,bool> ires =
            g_file_map.insert(pair<DWORD,FILE_STRUCT>(binKey,fs));
        if (!ires.second)
        {
            // replace existing entry
            TRACE(L"GetBinSize:: NOTICE: updating existing entry: Handles: (new)%04x vs (old)%04x", (DWORD)hfile, (DWORD)ires.first->second.hfile);
            // update existing entry (close old file handle, if different)
            if (hfile != ires.first->second.hfile)
                CloseHandle(ires.first->second.hfile);
            ires.first->second.hfile = hfile;
            ires.first->second.fsize = fsize;
            ires.first->second.offset = 0;
        }

        // modify buffer size
        result = fsize;
        TRACE(L"GetBinSize:: afsId=%02x, binId=%d, orgSize=%0x, newSize=%0x", 
                afsId, fileId, orgSize, result);
    }
    else {
        // insert a dummy object, so that we know
        // not to call the callbacks again
        FILE_STRUCT fs;
        fs.hfile = INVALID_HANDLE_VALUE;
        fs.fsize = 0;
        fs.offset = 0;
        fs.binKey = 0;

        DWORD binKey = (afsId << 16) + fileId;
        g_file_map[binKey] = fs;
    }
    return result;
}

void compareSpeed(const char* pathName)
{
    const int num_iterations = 2000000;
    int start = GetTickCount();
    for (int i=0; i<num_iterations; i++) {
        GetAfsIdByPathName(pathName);
    }
    int total = GetTickCount() - start;
    int startOld = GetTickCount();
    for (int i=0; i<num_iterations; i++) {
        GetAfsIdByPathNameOld(pathName);
    }
    int totalOld = GetTickCount() - startOld;
    int startFast = GetTickCount();
    for (int i=0; i<num_iterations; i++) {
        GetAfsIdByPathNameFast(pathName);
    }
    int totalFast = GetTickCount() - startFast;

    wchar_t* path = Utf8::utf8ToUnicode((BYTE*)pathName);
    LOG(L"compareSpeed:: pathName={%s}", path);
    LOG(L"compareSpeed:: total=%d, totalOld=%d, totalFast=%d", 
            total, totalOld, totalFast);
    Utf8::free(path);
}

DWORD GetAfsIdByPathName(const char* pathName)
{
    DWORD afsId = 0xffffffff;
    const char* start = strrchr(pathName,'\\')+1;
    const char* end = start+strlen(start);
    if (strnicmp(end-4,".img",4)==0) {
        if (sscanf(start+2,"%02x",&afsId)==1) {
            if (afsId<0 || afsId>MAX_AFSID) {
                LOG(L"GetAfsByPathName: afsId=%02x is out of range");
                return 0xffffffff;
            }
        }
    }
    return afsId;
}

DWORD GetAfsIdByPathNameOld(const char* pathName)
{
    AFS_INFO** ppBST = (AFS_INFO**)dta[BIN_SIZES_TABLE];
    for (DWORD afsId=0; afsId<=MAX_AFSID; afsId++)
    {
        if (ppBST[afsId]==0) continue;
        if (strncmp(ppBST[afsId]->filename,pathName,MAX_RELPATH)==0)
            return afsId;
    }
    return 0xffffffff;
}

/**
 * Optimized routine to determine
 * Best case of GetAfsIdByPathNameOld is faster, but
 * this method is more consistent in terms of speed.
 */
DWORD GetAfsIdByPathNameFast(const char* pathName)
{
    // quick exit check
    if (!pathName || pathName[0]=='\0') {
        return 0xffffffff;
    }

    DWORD afsId = 0xffffffff;
    const char* p = pathName+strlen(pathName)-1;
    const char* q1 = _imgExt1End;
    const char* q2 = _imgExt2End;
    int count = _imgExt1Len;
    while (*p!='\\' && p!=pathName) {
        // extension check
        if (count>0) {
            if (*p != *q1 && *p != *q2)
                return 0xffffffff;
            count--;
            q1--;
            q2--;
        }
        p--;
    }

    // parse afsId
    if (p == pathName) {
        return 0xffffffff;
    }
    if (sscanf(p+3,"%02x",&afsId)==1) {
        if (afsId<0 || afsId>MAX_AFSID) {
            LOG(L"GetAfsByPathNameFast: afsId=%02x is out of range");
            return 0xffffffff;
        }
    }

    return afsId;
}

KEXPORT DWORD GetAfsIdByBase(DWORD base)
{
    DWORD* binSizesTable = (DWORD*)dta[BIN_SIZES_TABLE];
    for (DWORD i=0; i<MAX_AFSID+1; i++)
        if (binSizesTable[i]==base)
            return i;
    return 0xffffffff;
}

KEXPORT DWORD GetAfsIdByReadEvent(READ_EVENT_STRUCT* res, const char* pathName)
{
    TRACE(L"GetAfsIdByReadEvent:: res->binSizesTableAddr = %p",
            res->binSizesTableAddr);
    DWORD* binSizesTable = (DWORD*)dta[BIN_SIZES_TABLE];
    for (DWORD i=0; i<MAX_AFSID+1; i++)
        if (binSizesTable[i]==res->binSizesTableAddr-0x10)
            return i;
    //return GetAfsIdByPathName(pathName);
    return GetAfsIdByPathNameFast(pathName);
}

void afsioAfterGetOffsetPagesCallPoint()
{
    __asm {
        // IMPORTANT: when saving flags, use pusfd/popfd for stack alignment
        pushfd 
        push ebp
        push eax
        push ebx
        push ecx
        push edx
        push esi
        push edi
        mov eax, [esp+0x24+0x40]
        cmp edi,eax
        je case1
        mov edx, [esp+0x24+0x08]
        jmp cont
case1:  mov edx, [esp+0x24+0x3c]
cont:   push edi // binId
        push edx // afsId
        push ebp // offset pages
        call afsioAfterGetOffsetPages
        add esp,0x0c     // pop parameters
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        mov edx, dword ptr ds:[esi+0x114]
        retn
    }
}

KEXPORT void afsioAfterGetOffsetPages(
        DWORD offsetPages, DWORD afsId, DWORD binId)
{
    TRACE(L"afsAfterGetOffsetPages:: offsetPages=%08x, afsId=%02x, binId=%d",
            offsetPages, afsId, binId);

    DWORD binKey = ((offsetPages << 0x0b)&0xfffff800) + afsId;
    g_offset_map[binKey] = binId;  // update existing entry, or insert new
}

void afsioAfterCreateEventCallPoint()
{
    __asm {
        // IMPORTANT: when saving flags, use pusfd/popfd for stack alignment
        pushfd 
        push ebp
        push eax
        push ebx
        push ecx
        push edx
        push esi
        push edi
        mov edx, [esp+0x24+0x0c]
        mov ecx, [esp+0x24+0x3c]
        push edx  // relative img-file pathname
        push ecx  // pointer to READ_EVENT_STRUCT
        push eax  // event id
        call afsioAfterCreateEvent
        add esp,0x0c     // pop parameters
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        mov dword ptr ds:[esi+0x27c],eax  // ...
        cmp eax,ebx                       // execute replaced code
        retn
    }
}

KEXPORT void afsioAfterCreateEvent(DWORD eventId, READ_EVENT_STRUCT* res, char* pathName)
{
    // The task here is to link the eventId with afsId/binId 
    // of the file that is going to be read later.
    //
#ifndef MYDLL_RELEASE_BUILD
    wchar_t* path = Utf8::utf8ToUnicode((BYTE*)pathName);
    TRACE(L"afsAfterCreateEvent:: eventId=%08x, pathName=%s",eventId,path);
    Utf8::free(path);
#endif

    //DWORD afsId = GetAfsIdByPathName(pathName);
    //DWORD afsId = GetAfsIdByReadEvent(res, pathName);
    DWORD afsId = GetAfsIdByPathNameFast(pathName);
    TRACE(L"afsAfterCreateEvent:: res=%p", res);
    TRACE(L"afsAfterCreateEvent:: afsId=%02x, offsetPages=%08x",
            afsId, res->offsetPages);
    DWORD binKey = ((res->offsetPages << 0x0b)&0xfffff800) + afsId;
    TRACE(L"afsAfterCreateEvent:: binKey=%08x",binKey);

    hash_map<DWORD,DWORD>::iterator it = g_offset_map.find(binKey);
    if (it != g_offset_map.end())
    {
        DWORD binId = it->second;

        /*
        if (afsId==0x02 && binId==45) {
            //__asm int 3;

            //TEST: compare speed
            compareSpeed(".\\img\\dt00_e.img");
            compareSpeed(".\\img\\dt0e.img");
            compareSpeed(".\\img\\dt00.img");
            compareSpeed("c:\\documents and settings\\all users\\application data\\konami\\pro evolution soccer 2011\\download\\dt0f.img");
        }
        */

        // lookup FILE_STRUCT
        DWORD binKey1 = (afsId << 16) + binId;
        TRACE(L"afsAfterCreateEvent:: looking for binKey1=%08x (afsId=%02x, binId=%d)",
                binKey1, afsId, binId);
        DWORD orgSize = 0; 
        hash_map<DWORD,FILE_STRUCT>::iterator fit = g_file_map.find(binKey1);
        if (fit == g_file_map.end()) {
            orgSize = GetBinSize(afsId, binId, orgSize);
            if (orgSize) {
                fit = g_file_map.find(binKey1);
            }
        }
        if (fit != g_file_map.end() && fit->second.fsize > 0)
        {
            // remember offset for later usage
            fit->second.offset = (res->offsetPages << 0x0b)&0xfffff800;

            // Found: so we have a replacement file handle
            // Now associate the eventId with that struct
            g_event_map.insert(pair<DWORD,FILE_STRUCT>(eventId,fit->second));

            TRACE(L"afsAfterCreateEvent:: eventId=%08x (afsId=%02x, binId=%d)",
                    eventId, afsId, binId);
        }

        // keep offset map small
        g_offset_map.erase(it);
    }
}

void afsioAtGetImgSize1CallPoint()
{
    __asm {
        pushfd 
        push ebp
        push eax
        push ebx
        push ecx
        push edx
        push esi
        push edi
        mov edx,0x7ffff800 // maxed-out size effectively
        mov [eax+8],edx    // execute replaced code
        mov dword ptr ds:[edi],edx // ...
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        retn
    }
}

void afsioAtGetImgSize2CallPoint()
{
    __asm {
        pushfd
        push eax
        mov eax,0x7ffff800   // maxed-out size effectively
        mov dword ptr ds:[esi+0x250],eax
        shr eax,0x0b
        mov dword ptr ds:[esi+0x258],eax
        pop eax
        popfd
        mov ecx, dword ptr ds:[esi+0x24c]  // execute replaced code
        retn
    }
}

void afsioBeforeReadCallPoint()
{
    __asm {
        // IMPORTANT: when saving flags, use pusfd/popfd for stack alignment
        pushfd 
        push ebp
        push eax
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push esi  // pointer to READ_STRUCT
        call afsioBeforeRead
        add esp,4  // pop parameters
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        sub esp,4                       // execute replaced code
        mov ecx,[esp+4]                 // (note: need to swap the values
        mov [esp],ecx                   // on top two slots on the stack)
        mov ecx,dword ptr ds:[esi+0x34] // ...
        mov [esp+4],ecx                 // ...
        jmp eax                         // ...
    }
}

void afsioAfsReadFileCallPoint()
{
    __asm {
        // IMPORTANT: when saving flags, use pusfd/popfd for stack alignment
        pushfd 
        push ebp
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push esi  // pointer to FILE_READ_STRUCT
        push 0    // lpOverlapped
        push eax  // lpNumberOfBytesRead
        push edi  // nNumberOfBytesToRead
        push ebx  // buffer
        push ecx  // hFile
        call AfsReadFile
        add esp,0x18  // pop parameters
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop ebp
        popfd
        push edx
        mov edx,[esp+4]
        mov [esp+0x18],edx
        pop edx
        add esp,0x14
        retn
    }
}

KEXPORT void afsioBeforeRead(READ_STRUCT* rs)
{
    TRACE(L"afsBeforeRead::(%08x) ...", (DWORD)rs->pfrs->eventId); 
    if (rs->pfrs->eventId == 0)
        return;

    hash_map<DWORD,FILE_STRUCT>::iterator it = g_event_map.find(rs->pfrs->eventId);
    if (it != g_event_map.end())
    {
        TRACE(L"afsBeforeRead::(%08x) WAS: hfile=%08x, offset=%08x", 
                (DWORD)rs->pfrs->eventId, 
                (DWORD)rs->pfrs->hfile, 
                rs->pfrs->offset); 

        FILE_STRUCT& fs = it->second;
        rs->pfrs->hfile = fs.hfile;
        rs->pfrs->offset -= fs.offset;
        rs->pfrs->offset_again -= fs.offset;

        TRACE(L"afsBeforeRead::(%08x) NOW: hfile=%08x, offset=%08x", 
                (DWORD)rs->pfrs->eventId, 
                (DWORD)rs->pfrs->hfile, 
                rs->pfrs->offset); 
        //if (fs.binKey == 0x00f07e2)
        //{
        //    _asm int 3;
        //}
    }
}

void afsioAtCloseHandleCallPoint()
{
    __asm {
        // IMPORTANT: when saving flags, use pusfd/popfd for stack alignment
        pushfd 
        push ebp
        push eax
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push eax // handle
        call afsioAtCloseHandle
        add esp,0x04     // pop parameters
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        push edx                          // execute replaced code
        mov edx,[esp+4]                   // (need to swap two numbers on
        mov [esp+4],eax                   // top of the stack, before doing
        mov [esp+8],edx                   // the call to CloseHandle)
        pop edx                           // ...
        call ebx                          // ...
        mov dword ptr ds:[esi+0x27c],edi  // ...
        retn
    }
}

KEXPORT void afsioAtCloseHandle(DWORD eventId)
{
    // reading event is complete. Handle is being closed
    // Do necessary house-keeping tasks, such as removing the eventId
    // from the event map
    TRACE(L"afsAtCloseHandle:: eventId=%08x",eventId);

    hash_map<DWORD,FILE_STRUCT>::iterator it = g_event_map.find(eventId);
    if (it != g_event_map.end())
    {
        FILE_STRUCT& fs = it->second;
        HANDLE hfile = fs.hfile;

        // delete entry in file_map
        hash_map<DWORD,FILE_STRUCT>::iterator fit = g_file_map.find(fs.binKey);
        if (fit != g_file_map.end())
        {
            TRACE(L"Finished read-event for afsId=0x%02x, binId=%d",
                    (fs.binKey >> 16)&0xffff, fs.binKey&0xffff);
            g_file_map.erase(fit);
        }

        // close file handle, and delete from event map
        CloseHandle(hfile);
        g_event_map.erase(it);

    }
}

KEXPORT bool afsioExtendSlots(int afsId, int num_slots)
{
    // extend BIN-sizes table
    AFS_INFO** tabArray = (AFS_INFO**)dta[BIN_SIZES_TABLE];
    if (!tabArray)
        return false;
    AFS_INFO* table = tabArray[afsId];
    if (!table)
        return false;

    // check if already enough slots
    if (num_slots <= table->numItems)
        return true;

    int newSize = sizeof(DWORD)*(num_slots)+0x120;
    AFS_INFO* newTable = (AFS_INFO*)HeapAlloc(
            GetProcessHeap(), HEAP_ZERO_MEMORY, newSize);
    memcpy(newTable, table, table->structSize);
    for (int i=table->numItems; i<num_slots; i++)
        newTable->sizes[i] = 0x800; // back-fill with 1 page defaults

    newTable->structSize = newSize;
    newTable->numItems = num_slots;
    newTable->numItems2 = num_slots;
    newTable->entryIsDword = 1;
    tabArray[afsId] = newTable; // point to new structure
    LOG(L"AFS img (0x%02x) extended to %d slots", afsId, num_slots);
    return true;
}

/**
 * Research function. Do not use yet.
 * Hooking:
 *  HookCallPoint(
 *      code[C_AT_READFILE], 
 *      afsioAfsReadFileCallPoint, 6, 1);
 *
 * code[C_AT_READFILE] = 0x4d0ba8 (for PES2011demo)
 *
 */
KEXPORT BOOL AfsReadFile(
        HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead,
        LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped,
        FILE_READ_STRUCT* pfrs)
{
    if (!pfrs) {
        // quick return
        return ReadFile(hFile, lpBuffer, nNumberOfBytesToRead,
                lpNumberOfBytesRead, lpOverlapped); 
    }

    TRACE(L"AfsReadFile: hFile=%04x, buffer=%p, offset=%08x, nb=%08x, %d",
            hFile, lpBuffer, pfrs->offset, nNumberOfBytesToRead, lpOverlapped);

    if (pfrs->eventId != 0)
    {
        TRACE(L"eventId = %04x", pfrs->eventId);
        hash_map<DWORD,FILE_STRUCT>::iterator it;
        it = g_event_map.find(pfrs->eventId);
        if (it != g_event_map.end())
        {
            TRACE(L"Event found!");
            FILE_STRUCT& fs = it->second;
            TRACE(L"AfsReadFile: fs.hfile=%04x, offset=%08x, nb=%08x",
                    fs.hfile, pfrs->offset-fs.offset, 
                    nNumberOfBytesToRead);

            SetFilePointer(fs.hfile, pfrs->offset-fs.offset, NULL, FILE_BEGIN);
            ReadFile(fs.hfile, lpBuffer, nNumberOfBytesToRead,
                    lpNumberOfBytesRead, lpOverlapped); 

            SetFilePointer(hFile, nNumberOfBytesToRead, NULL, FILE_CURRENT);
            if (lpNumberOfBytesRead) {
                *lpNumberOfBytesRead = nNumberOfBytesToRead;
            }
            return TRUE;
        }
    }

    // read afs data
    return ReadFile(hFile, lpBuffer, nNumberOfBytesToRead,
            lpNumberOfBytesRead, lpOverlapped); 
}

