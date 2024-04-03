#define UNICODE
#define THISMOD &k_camera

#include <windows.h>
#include <kload_exp.h>
#include "camera.h"
#include "camera_addr.h"
#include <dllinit.h>
#include <configs.h>

KMOD k_camera = {MODID, NAMELONG, NAMESHORT, DEFAULT_DEBUG};
camera* cams;
BYTE camPos;

HRESULT STDMETHODCALLTYPE initModule(IDirect3D9* self, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters,
    IDirect3DDevice9** ppReturnedDeviceInterface);

void cameraConfig(char* pName, const void* pValue, DWORD a);

EXTERN_C BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    switch(dwReason)
    {
        case DLL_PROCESS_ATTACH:
            RegisterKModule(THISMOD);
            if (!checkGameVersion()) 
            {
                LOG(L"Sorry, your game version isn't supported!");
                return false;
            }

            CHECK_KLOAD(MAKELONG(0, KITSERVER_VERSION_MAJOR));
            copyAdresses();
            hookFunction(hk_D3D_CreateDevice, initModule);
            break;

        case DLL_PROCESS_DETACH:
            LOG(L"Detaching dll...");
            break;
    }

    return TRUE;
}

HRESULT STDMETHODCALLTYPE initModule(IDirect3D9* self, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters,
    IDirect3DDevice9** ppReturnedDeviceInterface) 
{
    unhookFunction(hk_D3D_CreateDevice, initModule);
    
    LOG(L"Initializing Camera Settings");

    cams = (camera*) dta[CAMERA_SETTINGS_TABLE];
    if(cams)
    {
        const char* sectionName = "camera";
        char keyName[30];
        // Read & apply cameras settings from the config file
        for (camPos = 0; camPos < dta[NUM_CAMS]; camPos++, cams++)
        {
            if (!ableCams[camPos])
                continue;

            strcpy(keyName, camsNames[camPos]);
            strcat(keyName, ".range");
            getConfig(sectionName, keyName, DT_FLOAT, 1, cameraConfig);
            strcpy(keyName, camsNames[camPos]);
            strcat(keyName, ".height");
            getConfig(sectionName, keyName, DT_FLOAT, 2, cameraConfig);
            strcpy(keyName, camsNames[camPos]);
            strcat(keyName, ".zoom.near");
            getConfig(sectionName, keyName, DT_FLOAT, 3, cameraConfig);
            strcpy(keyName, camsNames[camPos]);
            strcat(keyName, ".zoom.far");
            getConfig(sectionName, keyName, DT_FLOAT, 4, cameraConfig);
            strcpy(keyName, camsNames[camPos]);
            strcat(keyName, ".camera.angle");
            getConfig(sectionName, keyName, DT_FLOAT, 5, cameraConfig);
            strcpy(keyName, camsNames[camPos]);
            strcat(keyName, ".angle.modulator");
            getConfig(sectionName, keyName, DT_FLOAT, 6, cameraConfig);
        }
    }
       TRACE(L"Initialization complete.");
    return D3D_OK;
}


void cameraConfig(char* pName, const void* pValue, DWORD a)
{
    wchar_t temp[30];
    switch (a) {
        case 1:	// Camera range
            mbstowcs(temp, camsNames[camPos], 30);
            LOG(L"%ls.range: %f", temp, *(float*)pValue);
            Patch(&cams->range, pValue, sizeof(float));
            break;
        case 2: // Camera height
            mbstowcs(temp, camsNames[camPos], 30);
            LOG(L"%ls.height: %f", temp, *(float*)pValue);
            Patch(&cams->height, pValue, sizeof(float));
            break;
        case 3: // Camera zoom.near
            mbstowcs(temp, camsNames[camPos], 30);
            LOG(L"%ls.zoom.near: %f", temp, *(float*)pValue);
            Patch(&cams->zoomNear, pValue, sizeof(float));
            break;
        case 4: // Camera zoom.far
            mbstowcs(temp, camsNames[camPos], 30);
            LOG(L"%ls.zoom.far: %f", temp, *(float*)pValue);
            Patch(&cams->zoomFar, pValue, sizeof(float));
            break;
        case 5: // Camera camera.angle
            mbstowcs(temp, camsNames[camPos], 30);
            LOG(L"%ls.camera.angle: %f", temp, *(float*)pValue);
            Patch(&cams->cameraAngle, pValue, sizeof(float));
            break;
        case 6: // Camera angle.modulator
            mbstowcs(temp, camsNames[camPos], 30);
            LOG(L"%ls.angle.modulator: %f", temp, *(float*)pValue);
            Patch(&cams->angleModulator, pValue, sizeof(float));
            break;
    }
}