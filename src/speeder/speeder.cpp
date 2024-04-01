/* <speeder>
 *
 */
#define UNICODE
#define THISMOD &k_speed

#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <sys/stat.h>
#include "kload_exp.h"
#include "speeder.h"
#include "speeder_addr.h"
#include "dllinit.h"
#include "configs.h"
#include "configs.hpp"
#include "utf8.h"

//#include "apihijack.h"

#define lang(s) getTransl("speeder",s)

//#include <map>
//#include <list>
//#include <hash_map>
//#include <wchar.h>

#define SWAPBYTES(dw) \
    ((dw<<24 & 0xff000000) | (dw<<8  & 0x00ff0000) | \
    (dw>>8  & 0x0000ff00) | (dw>>24 & 0x000000ff))

#define round(x) ((abs((x)-(int)(x))<0.5)?(int)(x):(int)((x)+1))

#define FLOAT_ZERO 0.0001f
#define MIN_COUNT 0.2
#define MAX_COUNT 2.5
#define DBG(x) {if (k_speed.debug) x;}

// VARIABLES
HINSTANCE hInst = NULL;
KMOD k_speed = {MODID, NAMELONG, NAMESHORT, DEFAULT_DEBUG};

// GLOBALS

KEXPORT BOOL WINAPI Override_QueryPerformanceFrequency(
        LARGE_INTEGER *lpPerformanceFrequency);

bool _initialized(false);
LARGE_INTEGER _metric;

class config_t 
{
public:
    config_t() : count_factor(1.0) {}
    float count_factor;
};

config_t _speeder_config;

// FUNCTIONS
HRESULT STDMETHODCALLTYPE initModule(IDirect3D9* self, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters, 
    IDirect3DDevice9** ppReturnedDeviceInterface);

void speederConfig(char* pName, const void* pValue, DWORD a);
void realInitModule();


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

void speederConfig(char* pName, const void* pValue, DWORD a)
{
    HANDLE handle;
    DWORD fsize;

	switch (a) {
		case 1: // debug
			k_speed.debug = *(DWORD*)pValue;
			break;
        case 2: // tick-count factor
			_speeder_config.count_factor = *(float*)pValue;
            // sanity checks
            if (_speeder_config.count_factor < MIN_COUNT)
                _speeder_config.count_factor = MIN_COUNT;
            if (_speeder_config.count_factor > MAX_COUNT)
                _speeder_config.count_factor = MAX_COUNT;
            break;
	}
	return;
}

HRESULT STDMETHODCALLTYPE initModule(IDirect3D9* self, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters, 
    IDirect3DDevice9** ppReturnedDeviceInterface) 
{
    LOG(L"Initializing Speeder Module");

    getConfig("speeder", "debug", DT_DWORD, 1, speederConfig);
    getConfig("speeder", "count.factor", DT_FLOAT, 2, speederConfig);

    LOG(L"configuration: count.factor = %0.3f",
                _speeder_config.count_factor);

    realInitModule();
	TRACE(L"Initialization complete.");
    return D3D_OK;
}

void realInitModule()
{
	unhookFunction(hk_D3D_CreateDevice, initModule);

    // patch code to enforce specified time
    if (_speeder_config.count_factor >= MIN_COUNT &&
            fabs(_speeder_config.count_factor-1.0) > FLOAT_ZERO) {
        LOG(L"speed adjustment enabled");

        // hook
        /*
        SDLLHook kernel32Hook = 
        {
            "KERNEL32.DLL",
            false, NULL, // Default hook disabled, NULL function pointer.
            {
                { 
                    "QueryPerformanceFrequency", 
                    Override_QueryPerformanceFrequency 
                },
                { NULL, NULL }
            }
        };
        HookAPICalls( &kernel32Hook );
        */
        BOOL result = QueryPerformanceFrequency(&_metric);
        LOG(L"old metric(%u, %u), result=%d", 
           _metric.HighPart, _metric.LowPart, result);
        _metric.QuadPart /= _speeder_config.count_factor;
        LOG(L"new metric(%u, %u)", 
           _metric.HighPart, _metric.LowPart);

        HookIndirectCall2(code[CS_QUERY_PERFORMANCE_FREQUENCY], 
            Override_QueryPerformanceFrequency);
    }
}

KEXPORT BOOL WINAPI Override_QueryPerformanceFrequency(
        LARGE_INTEGER *lpPerformanceFrequency)
{
    //LOG(L"called QueryPerformanceFrequency!");
    
    //BOOL result = QueryPerformanceFrequency(&_metric);
    //_metric.QuadPart /= _speeder_config.count_factor;

    *lpPerformanceFrequency = _metric;
    //return result;
    return TRUE;
}

