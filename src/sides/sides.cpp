/* <sides>
 *
 */
#define UNICODE
#define THISMOD &k_sides

#include <windows.h>
#include <stdio.h>
#include <sys/stat.h>
#include "kload_exp.h"
#include "sides.h"
#include "sides_addr.h"
#include "dllinit.h"
#include "configs.h"
#include "configs.hpp"
#include "utf8.h"

//#define lang(s) getTransl("sides",s)

//#include <map>
//#include <list>
//#include <hash_map>
//#include <wchar.h>

#define SWAPBYTES(dw) \
    ((dw<<24 & 0xff000000) | (dw<<8  & 0x00ff0000) | \
    (dw>>8  & 0x0000ff00) | (dw>>24 & 0x000000ff))

#define round(x) ((abs((x)-(int)(x))<0.5)?(int)(x):(int)((x)+1))

#define FLOAT_ZERO 0.0001f
#define DBG(x) {if (k_sides.debug) x;}

// VARIABLES
HINSTANCE hInst = NULL;
KMOD k_sides = {MODID, NAMELONG, NAMESHORT, DEFAULT_DEBUG};

// GLOBALS

class config_t 
{
public:
    config_t() : _free_select(false) {}
    bool _free_select;
};

config_t _sides_config;

// FUNCTIONS
HRESULT STDMETHODCALLTYPE initModule(IDirect3D9* self, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters, 
    IDirect3DDevice9** ppReturnedDeviceInterface);

void sidesConfig(char* pName, const void* pValue, DWORD a);
KEXPORT BOOL WINAPI Override_QueryPerformanceFrequency(
        LARGE_INTEGER *lpPerformanceFrequency);


/*******************/
/* DLL Entry Point */
/*******************/
EXTERN_C BOOL WINAPI DllMain(
        HINSTANCE hInstance, 
        DWORD dwReason, 
        LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		hInst = hInstance;

        RegisterKModule(THISMOD);
        if (!checkGameVersion()) {
            LOG(L"Sorry, your game version isn't supported!");
            return false;
        }

        CHECK_KLOAD(MAKELONG(0, KITSERVER_VERSION_MAJOR));

        copyAdresses();
		hookFunction(hk_D3D_CreateDevice, initModule);
	}
	
	else if (dwReason == DLL_PROCESS_DETACH)
	{
	}
	
	return true;
}

void sidesConfig(char* pName, const void* pValue, DWORD a)
{
    HANDLE handle;
    DWORD fsize;

	switch (a) {
		case 1: // debug
			k_sides.debug = *(DWORD*)pValue;
			break;
        case 2: // tick-count factor
			_sides_config._free_select = (*(DWORD*)pValue != 0);
            break;
	}
	return;
}

HRESULT STDMETHODCALLTYPE initModule(IDirect3D9* self, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters, 
    IDirect3DDevice9** ppReturnedDeviceInterface) {

	unhookFunction(hk_D3D_CreateDevice, initModule);

    LOG(L"Initializing Sides Module");

    getConfig("sides", "debug", DT_DWORD, 1, sidesConfig);
    getConfig("sides", "free.select", DT_DWORD, 2, sidesConfig);

    LOG(L"configuration: free.select = %d",
                _sides_config._free_select);

    // patch code to enable free side selection
    if (_sides_config._free_select) {
		DWORD newProtection = PAGE_EXECUTE_READWRITE;
		DWORD savedProtection;
        BYTE* p;
        
        p = (BYTE*)code[CS1];
		if (VirtualProtect(p, 8, newProtection, &savedProtection)) {
            memcpy(p, "\xeb\x76\x90", 3); // jmp short 510b7f
		}
        p = (BYTE*)code[CS2];
		if (VirtualProtect(p, 8, newProtection, &savedProtection)) {
            memcpy(p, "\xba\0\0\0\0\x90", 6);  // mov edx,0
		}
        p = (BYTE*)code[CS3];
		if (VirtualProtect(p, 8, newProtection, &savedProtection)) {
            memcpy(p, "\xb9\0\0\0\0\x90", 6);  // mov ecx,0
		}
        p = (BYTE*)code[CS4];
		if (VirtualProtect(p-6, 8, newProtection, &savedProtection)) {
            *p = 0;  // cmp [eax+0x140],3 --> cmp [eax+0x140],0
		}
    }

	TRACE(L"Initialization complete.");
    return D3D_OK;
}

