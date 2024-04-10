/* Kitserver EXE Manager */

#define UNICODE

#include <windows.h>
#include <windef.h>
#include <stdio.h>
#include "utf8.h"

#include <map>
#include <string>

#define DLL_NAME "kload\0"
#define DLL_NAME_SET "kset\0"
#define DEFAULT_EXE_NAME L"pes2013\0"
#define DEFAULT_DEMO_EXE_NAME L"pes2013demo.exe\0"
#define BUFLEN 4096
#define SW sizeof(wchar_t)
#define WBUFLEN (SW * BUFLEN)

#include "imageutil.h"
#include "setupgui.h"
#include "manager.h"
#include "detect.h"
#include "configs.h"
#include "lang.h"
#include "manage.h"
#include "vstyles.h"
#define lang(s) _getTransl("manager",s)

HWND hWnd = NULL;
bool g_noFiles = false;
wchar_t mydir[BUFLEN];
char patchFolderName[BUFLEN]={0};
wchar_t patchExeName[BUFLEN]={0};
wchar_t patchTitle[BUFLEN]={0};
char installDllPath[BUFLEN]={0};
char installDllSetPath[BUFLEN]={0};
wchar_t setupLang[64] = L"eng\0";

// array of LoadLibrary address on diff. OS
DWORD LoadLibraryAddr[] = {
	0,            // auto-detect
	0x77e7d961,   // WinXP
	0x7c5768fb,   // Win2K 
};

bool InstallKserv();
bool InstallKserv(wstring& gfile, wstring& sfile, wstring& outs, const bool quiet=true);
bool RemoveKserv();
bool RemoveKserv(wstring& gfile, wstring& sfile, wstring& outs, const bool quiet=true);
void MyMessageBox(wchar_t* fmt, DWORD value);
void MyMessageBox2(wchar_t* fmt, wchar_t* value);
void UpdateInfo(void);

void MyMessageBox(wchar_t* fmt, DWORD value)
{
#ifndef MYDLL_RELEASE_BUILD
	// show message box with error msg
	wchar_t buf[BUFLEN];
	ZeroMemory(buf, WBUFLEN);
	swprintf(buf, fmt, value);
	MessageBox(hWnd, buf, L"Kitserver DEBUG MyMessage", 0);
#endif
}

void MyMessageBox2(wchar_t* fmt, wchar_t* value)
{
#ifndef MYDLL_RELEASE_BUILD
	// show message box with error msg
	wchar_t buf[BUFLEN];
	ZeroMemory(buf, WBUFLEN);
	swprintf(buf, fmt, value);
	MessageBox(hWnd, buf, L"Kitserver DEBUG MyMessage", 0);
#endif
}

void setupConfig(char* pName, const void* pValue, DWORD a) {
	switch (a) {
		case 1:	// lang
			wcscpy(setupLang, (wchar_t*)pValue);
			break;
	}
}

/**
 * Installs the kitserver DLL.
 */
bool InstallKserv()
{
    wchar_t fileName[2][BUFLEN];

	for (int i=0; i<2; i++) {
		HWND listControl = (i==0)?g_exeListControl:g_setListControl;

		ZeroMemory(fileName[i], WBUFLEN);
		wcscpy(fileName[i], L"..\\");
		wchar_t* p = fileName[i] + lstrlen(fileName[i]);

        // get currently selected item and its text
		int idx = (int)SendMessage(listControl, CB_GETCURSEL, 0, 0);
        if (idx==0) wcscpy(fileName[i],L"");
        else SendMessage(listControl, CB_GETLBTEXT, idx, (LPARAM)p);
    }
	
    wstring outs;
    return InstallKserv(wstring(fileName[0]), wstring(fileName[1]), outs, false);
}

/**
 * Installs the kitserver DLL.
 */
bool InstallKserv(wstring& gfile, wstring& sfile, wstring& outs, const bool quiet)
{
    bool result = true;

	// disable buttons, 'cause it may take some time
	EnableWindow(g_installButtonControl, FALSE);
	EnableWindow(g_removeButtonControl, FALSE);
	
	wchar_t outmsg[1024];
	ZeroMemory(outmsg, SW * 1024);
	
	for (int i=0; i<2; i++) {
		HWND listControl = (i==0)?g_exeListControl:g_setListControl;
		wcscat(outmsg, (i==0)?L">>> Game EXE:\n":L"\n\n>>> Settings EXE:\n");

		wchar_t* fileName = (wchar_t*)((i==0)?gfile.c_str():sfile.c_str());
        if (wcslen(fileName)==0) {
            wcscat(outmsg, lang("NoActionRequired"));
            continue;
        }
	
		// check if it's a recognizable EXE-file
		if (GetRealGameVersion(fileName) == -1)
		{
			// show message box with error msg
			wchar_t buf[BUFLEN];
			ZeroMemory(buf, WBUFLEN);
			swprintf(buf, lang("Err_UnknownExe"), fileName);
	
			wcscat(outmsg, buf);
            result = false;
			continue;
		}
		
		if (isRealGame(GetRealGameVersion(fileName)) != (i==0)) {
			wchar_t buf[BUFLEN];
			ZeroMemory(buf, WBUFLEN);
			swprintf(buf, lang("Err_WrongExeType"), 
                    fileName, 
                    (i==0)?lang("ParamSettings"):lang("ParamGame"),
                    (i==0)?lang("ParamGame"):lang("ParamSettings")
                    );
			wcscat(outmsg, buf);
            result = false;
			continue;
		}
	
		// get the address of LoadLibrary function, which is different 
		// Here, we're taking advantage of the fact that Kernel32.dll, 
		// which contains the LoadLibrary function is never relocated, so 
		// the address of LoadLibrary will always be the same.
		HMODULE krnl = GetModuleHandle(L"kernel32.dll");
		DWORD loadLib = (DWORD)GetProcAddress(krnl, "LoadLibraryA");
		// Well, this doesn't work for Vista, so we don't determine the
		// address ourselves but use the one from the import table, for
		// which we get the pointer with the getImportThunkRVA command
	
	
		DWORD ep, ib;
		DWORD dataOffset, dataVA;
		DWORD codeOffset, codeVA;
		DWORD loadLibAddr, kservAddr, loadLibAddr1;
		DWORD newEntryPoint;
	
		FILE* f = _wfopen(fileName, L"r+b");
		if (f != NULL)
		{
			// Install
			loadLibAddr1 = getImportThunkRVA(f, "kernel32.dll","LoadLibraryA");
			
			// this used to cause the latest crashes on game start
			if (!loadLibAddr1) {
				// show message box with error msg
				wchar_t buf[BUFLEN];
				ZeroMemory(buf, WBUFLEN);
				swprintf(buf, L"\
======== ERROR! =========\n\
Couldn't find LoadLibraryA in\n\
%s.", fileName);

				wcscat(outmsg, buf);
                result = false;
				continue;
			}
			
			if (SeekEntryPoint(f))
			{
				fread(&ep, sizeof(DWORD), 1, f);
				//wprintf("Entry point: %08x\n", ep);
			}
			if (SeekImageBase(f))
			{
				fread(&ib, sizeof(DWORD), 1, f);
				//wprintf("Image base: %08x\n", ib);
			}
	
			IMAGE_SECTION_HEADER dataHeader;
			ZeroMemory(&dataHeader, sizeof(IMAGE_SECTION_HEADER));
			dataOffset = 0;
			dataVA = 0;
	
			// find empty space at the end of .rdata
			bool rdataFound(false);
            if (GetGameVersion(GetRealGameVersion(fileName)) >= gvPES2013demo) {
                rdataFound = SeekSectionHeader(f,".data");
            } 
            else {
                rdataFound = SeekSectionHeader(f, ".rdata");
            }
			if (!rdataFound) rdataFound = SeekSectionHeader(f, ".rsrc");
			if (!rdataFound) rdataFound = SeekSectionHeader(f, ".IN");
			if (rdataFound) {
				fread(&dataHeader, sizeof(IMAGE_SECTION_HEADER), 1, f);
	
	            //adjust SizeOfRawData (needed for WE9LEK-nocd)
	            int rem = dataHeader.SizeOfRawData % 0x80;
	            if (rem > 0) {
	                dataHeader.SizeOfRawData += 0x80 - rem;
	                fseek(f, -sizeof(IMAGE_SECTION_HEADER), SEEK_CUR);
	                fwrite(&dataHeader, sizeof(IMAGE_SECTION_HEADER), 1, f);
	            }
	
				dataVA = dataHeader.VirtualAddress + dataHeader.SizeOfRawData;
				if (dataHeader.PointerToRawData != 0)
					dataOffset = dataHeader.PointerToRawData + dataHeader.SizeOfRawData;
				else
					dataOffset = dataHeader.VirtualAddress + dataHeader.SizeOfRawData;
	
				// shift 28 bytes back
				dataOffset -= 0x1c;
				dataVA -= 0x1c;
			}
	
			if (!quiet) MyMessageBox(L"dataOffset = %08x", dataOffset);
			if (!quiet) MyMessageBox(L"dataVA = %08x", dataVA);
	
			if (dataOffset != 0) {
				// at the found empty place, write the LoadLibrary address, 
				// and the name of kserv.dll
				BYTE buf[0x1c], zero[0x1c];
				ZeroMemory(zero, 0x1c);
				ZeroMemory(buf, 0x1c);
	
				fseek(f, dataOffset, SEEK_SET);
				fread(&buf, 0x1c, 1, f);
				if (memcmp(buf, zero, 0x1c)==0)
				{
					// ok, we found an empty place. Let's live here.
					fseek(f, -0x1c, SEEK_CUR);
					DWORD* p = (DWORD*)buf;
					p[0] = ep; // save old entry pointer for easy uninstall
					p[1] = loadLib;
					if (i==0)
						memcpy(buf + 8, installDllPath, strlen(installDllPath)+1);
					else
						memcpy(buf + 8, installDllSetPath, strlen(installDllSetPath)+1);
					fwrite(buf, 0x1c, 1, f);
					
					loadLibAddr = ib + dataVA + sizeof(DWORD);
					//wprintf("loadLibAddr = %08x\n", loadLibAddr);
					kservAddr = loadLibAddr + sizeof(DWORD);
					//wprintf("kservAddr = %08x\n", kservAddr);
					
					loadLibAddr = ib + loadLibAddr1;
				}
				else
				{
					//wprintf("Already installed.\n");
					fclose(f);
	
					// show message box with error msg
					wchar_t buf[BUFLEN];
					ZeroMemory(buf, WBUFLEN);
					swprintf(buf, L"\
======== INFORMATION! =========\n\
Kitserver is already installed (1) for\n\
%s.", fileName);
	
					wcscat(outmsg, buf);
					continue;
				}
			}
	
			IMAGE_SECTION_HEADER textHeader;
			ZeroMemory(&textHeader, sizeof(IMAGE_SECTION_HEADER));
			codeOffset = 0;
			codeVA = 0;
	
			// find empty space at the end of .text
            bool textFound(false);
            if (GetGameVersion(GetRealGameVersion(fileName)) >= gvPES2013demo) {
                textFound = SeekSectionHeader(f,"Sitext");
                if (!textFound) textFound = SeekSectionHeader(f,"PSFD00");
            }
            else {
	            textFound = SeekSectionHeader(f, ".text");
            }
	        if (!textFound) textFound = SeekSectionHeader(f,"rr02");
	        if (!textFound) textFound = SeekSectionHeader(f,".HEAVEN");
	        if (!textFound) textFound = SeekSectionHeader(f,"");
			if (textFound) {
				fread(&textHeader, sizeof(IMAGE_SECTION_HEADER), 1, f);
	
	            //adjust SizeOfRawData (needed for WE9LEK-nocd)
	            int rem = textHeader.SizeOfRawData % 0x40;
	            if (rem > 0) {
	                textHeader.SizeOfRawData += 0x40 - rem;
	                fseek(f, -sizeof(IMAGE_SECTION_HEADER), SEEK_CUR);
	                fwrite(&textHeader, sizeof(IMAGE_SECTION_HEADER), 1, f);
	            }

                MyMessageBox(
                    L"VirtualAddress = %08x", textHeader.VirtualAddress);
                MyMessageBox(
                    L"SizeOfRawData = %08x", textHeader.SizeOfRawData);
	
				codeVA = textHeader.VirtualAddress + textHeader.SizeOfRawData;
				if (textHeader.PointerToRawData != 0)
					codeOffset = textHeader.PointerToRawData + textHeader.SizeOfRawData;
				else
					codeOffset = textHeader.VirtualAddress + textHeader.SizeOfRawData;
	
				// shift 32 bytes back.
				codeOffset -= 0x20;
				codeVA -= 0x20;
                if (GetGameVersion(GetRealGameVersion(fileName)) == gvPES2013demo) {
                    // shift more
                    codeOffset -= 0x20;
                    codeVA -= 0x20;
                }
			} else {
	            if (!quiet) MyMessageBox(L"section header for '.text' not found", 0);
                result = false;
	        }
	
			if (!quiet) MyMessageBox(L"codeOffset = %08x", codeOffset);
			if (!quiet) MyMessageBox(L"codeVA = %08x", codeVA);
	
			if (codeOffset != 0) {
				// at the found place, write the new entry point logic
				BYTE buf[0x20], zero[0x20];
				ZeroMemory(zero, 0x20);
				ZeroMemory(buf, 0x20);
	
				fseek(f, codeOffset, SEEK_SET);
				fread(&buf, 0x20, 1, f);
				if (memcmp(buf, zero, 0x20)==0)
				{
					// ok, we found an empty place. Let's live here.
					fseek(f, -0x20, SEEK_CUR);
					buf[0] = 0x68;  // push
					DWORD* p = (DWORD*)(buf + 1); p[0] = kservAddr;
					buf[5] = 0xff; buf[6] = 0x15; // call
					p = (DWORD*)(buf + 7); p[0] = loadLibAddr;
					buf[11] = 0xe9; // jmp
					p = (DWORD*)(buf + 12); p[0] = ib + ep - 5 - (ib + codeVA + 11);
					fwrite(buf, 0x20, 1, f);
	
					newEntryPoint = codeVA;
				}
                else if (memcmp(buf, "\xb9\x0c\x58\x4c", 4)==0)
                {
                    // special case of PES2009 settings.exe (full game)
                    codeOffset -= 0x1f;
                    codeVA -= 0x1f;

                    fseek(f, codeOffset, SEEK_SET);
					buf[0] = 0x68;  // push
					DWORD* p = (DWORD*)(buf + 1); p[0] = kservAddr;
                    buf[5] = 0xeb; buf[6] = 0x09; // jmp short to call
					buf[0x10] = 0xff; buf[0x11] = 0x15; // call
					p = (DWORD*)(buf + 0x12); p[0] = loadLibAddr;
					buf[0x16] = 0xe9; // jmp
					p = (DWORD*)(buf + 0x17); p[0] = ib + ep - 5 - (ib + codeVA + 0x16);
					fwrite(buf, 7, 1, f);
                    fseek(f, codeOffset+0x10, SEEK_SET);
					fwrite(buf+0x10, 11, 1, f);
	
					newEntryPoint = codeVA;
                }
				else
				{
					//wprintf("Already installed.\n");
					fclose(f);
	
					// show message box with error msg
					wchar_t mbuf[BUFLEN];
					ZeroMemory(mbuf, WBUFLEN);
					swprintf(mbuf, L"\
======== INFORMATION! =========\n\
Kitserver is already installed (2) for\n\
%s.", fileName);
	
					wcscat(outmsg, mbuf);
					continue;
				}
			}
			if (SeekEntryPoint(f))
			{
				// write new entry point
				fwrite(&newEntryPoint, sizeof(DWORD), 1, f);
				//wprintf("New entry point: %08x\n", newEntryPoint);
			}
	        /*
			if (SeekCodeSectionFlags(f))
			{
				DWORD flags;
				fread(&flags, sizeof(DWORD), 1, f);
				flags |= 0x80000000; // make code section writeable
				fseek(f, -sizeof(DWORD), SEEK_CUR);
				fwrite(&flags, sizeof(DWORD), 1, f);
			}
	        */
			fclose(f);
	
			// show message box with success msg
			wchar_t buf[BUFLEN];
			ZeroMemory(buf, WBUFLEN);
			swprintf(buf, L"\
======== SUCCESS! =========\n\
Setup has installed Kitserver for\n\
%s.", fileName);
	
			wcscat(outmsg, buf);
			continue;
		}
		else
		{
			// show message box with error msg
			wchar_t buf[BUFLEN];
			ZeroMemory(buf, WBUFLEN);
			swprintf(buf, L"\
======== ERROR! =========\n\
Setup failed to install Kitserver for\n\
%s.\n\
\n\
(No modifications made.)\n\
Verify that the executable is not\n\
READ-ONLY, and try again.", fileName);
	
			wcscat(outmsg, buf);
            result = false;
			continue;
		}
		
	}
	
	UpdateInfo();
	
	if (!quiet) MessageBox(hWnd, outmsg, lang("MsgTitle"), 0);	
    outs += outmsg;
    return result;
}

/**
 * Uninstalls the kitserver DLL.
 */
bool RemoveKserv()
{
    wchar_t fileName[2][BUFLEN];

	for (int i=0; i<2; i++) {
		HWND listControl = (i==0)?g_exeListControl:g_setListControl;

		ZeroMemory(fileName[i], WBUFLEN);
		wcscpy(fileName[i], L"..\\");
		wchar_t* p = fileName[i] + lstrlen(fileName[i]);
	
		// get currently selected item and its text
		int idx = (int)SendMessage(listControl, CB_GETCURSEL, 0, 0);
        if (idx==0) wcscpy(fileName[i],L"");
        else SendMessage(listControl, CB_GETLBTEXT, idx, (LPARAM)p);
    }

    wstring outs;
    return RemoveKserv(wstring(fileName[0]), wstring(fileName[1]), outs, false);
}

/**
 * Uninstalls the kitserver DLL.
 */
bool RemoveKserv(wstring& gfile, wstring& sfile, wstring& outs, const bool quiet)
{
    bool result = true;

	// disable buttons, 'cause it may take some time
	EnableWindow(g_installButtonControl, FALSE);
	EnableWindow(g_removeButtonControl, FALSE);

	wchar_t outmsg[1024];
	ZeroMemory(outmsg, SW * 1024);
	
	for (int i=0; i<2; i++) {
		HWND listControl = (i==0)?g_exeListControl:g_setListControl;
		wcscat(outmsg, (i==0)?L">>> Game EXE:\n":L"\n\n>>> Settings EXE:\n");

		wchar_t* fileName = (wchar_t*)((i==0)?gfile.c_str():sfile.c_str());
        if (wcslen(fileName)==0) {
            wcscat(outmsg, lang("NoActionRequired"));
            continue;
        }

		DWORD ep, ib;
		DWORD dataOffset, dataVA;
		DWORD codeOffset, codeVA;
		DWORD loadLibAddr, kservAddr;
		DWORD newEntryPoint;
	
		FILE* f = _wfopen(fileName, L"r+b");
		if (f != NULL)
		{
			if (SeekEntryPoint(f))
			{
				fread(&ep, sizeof(DWORD), 1, f);
				//wprintf("Current entry point: %08x\n", ep);
			}
	
			IMAGE_SECTION_HEADER dataHeader;
			ZeroMemory(&dataHeader, sizeof(IMAGE_SECTION_HEADER));
			dataOffset = 0;
			dataVA = 0;
	
			// find empty space at the end of .rdata
			bool rdataFound(false);
            if (GetGameVersion(GetRealGameVersion(fileName)) >= gvPES2013demo) {
                rdataFound = SeekSectionHeader(f,".data");
            } 
            else {
                rdataFound = SeekSectionHeader(f, ".rdata");
            }
            if (!rdataFound) rdataFound = SeekSectionHeader(f, ".rsrc");
            if (!rdataFound) rdataFound = SeekSectionHeader(f, ".IN");
			if (rdataFound) {
				fread(&dataHeader, sizeof(IMAGE_SECTION_HEADER), 1, f);
	
				dataVA = dataHeader.VirtualAddress + dataHeader.SizeOfRawData;
				if (dataHeader.PointerToRawData != 0)
					dataOffset = dataHeader.PointerToRawData + dataHeader.SizeOfRawData;
				else
					dataOffset = dataHeader.VirtualAddress + dataHeader.SizeOfRawData;
	
				// shift 28 bytes back
				dataOffset -= 0x20;
				dataVA -= 0x20;
			}
	
			if (!quiet) MyMessageBox(L"dataOffset = %08x", dataOffset);
			if (!quiet) MyMessageBox(L"dataVA = %08x", dataVA);
	
			if (dataOffset != 0) {
				// if already installed, this location should contain
				// some saved data.
				BYTE zero[0x20];
				ZeroMemory(zero, 0x20);
				fseek(f, dataOffset, SEEK_SET);
	
				// read saved old entry point
				fread(&newEntryPoint, sizeof(DWORD), 1, f);
                size_t writeSize = 0x20;
                if (newEntryPoint == 0) {
                    // try 0x1c
                    dataOffset += 4;
                    dataVA += 4;
                    fseek(f, dataOffset, SEEK_SET);
                    fread(&newEntryPoint, sizeof(DWORD), 1, f);
                    writeSize -= 4;
                }

				if (newEntryPoint == 0)
				{
					//wprintf("Already uninstalled.\n");
					fclose(f);
	
					// show message box with error msg
					wchar_t buf[BUFLEN];
					ZeroMemory(buf, WBUFLEN);
					swprintf(buf, L"\
======== INFORMATION! =========\n\
Kitserver is not attached to\n\
%s.", fileName);
	
					wcscat(outmsg, buf);
					continue;
				}
				// zero out the bytes
				fseek(f, -sizeof(DWORD), SEEK_CUR);
				fwrite(zero, writeSize, 1, f);
			}
	
			IMAGE_SECTION_HEADER textHeader;
			ZeroMemory(&textHeader, sizeof(IMAGE_SECTION_HEADER));
			codeOffset = 0;
			codeVA = 0;
	
			// find empty space at the end of .text
            bool textFound(false);
            if (GetGameVersion(GetRealGameVersion(fileName)) >= gvPES2013demo) {
                textFound = SeekSectionHeader(f,"Sitext");
                if (!textFound) textFound = SeekSectionHeader(f,"PSFD00");
            }
            else {
	            textFound = SeekSectionHeader(f, ".text");
            }
	        if (!textFound) textFound = SeekSectionHeader(f,"rr02");
	        if (!textFound) textFound = SeekSectionHeader(f,".HEAVEN");
	        if (!textFound) textFound = SeekSectionHeader(f,"");
			if (textFound) {
				fread(&textHeader, sizeof(IMAGE_SECTION_HEADER), 1, f);

                MyMessageBox(
                    L"VirtualAddress = %08x", textHeader.VirtualAddress);
                MyMessageBox(
                    L"SizeOfRawData = %08x", textHeader.SizeOfRawData);
	
				codeVA = textHeader.VirtualAddress + textHeader.SizeOfRawData;
				if (textHeader.PointerToRawData != 0)
					codeOffset = textHeader.PointerToRawData + textHeader.SizeOfRawData;
				else
					codeOffset = textHeader.VirtualAddress + textHeader.SizeOfRawData;
	
				// shift 32 bytes back.
				codeOffset -= 0x20;
				codeVA -= 0x20;
                if (GetGameVersion(GetRealGameVersion(fileName)) == gvPES2013demo) {
                    // shift more
                    codeOffset -= 0x20;
                    codeVA -= 0x20;
                }
			}
	
			if (!quiet) MyMessageBox(L"codeOffset = %08x", codeOffset);
			if (!quiet) MyMessageBox(L"codeVA = %08x", codeVA);
	
			if (codeOffset != 0) {
				// if installed, this should have the new entry point logic
				BYTE zero[0x20];
				BYTE buf[0x20];
				ZeroMemory(zero, 0x20);
				ZeroMemory(buf, 0x20);
	
				fseek(f, codeOffset, SEEK_SET);
				fread(buf, 0x20, 1, f);

                if (memcmp(buf, "\xb9\x0c\x58\x4c", 4)==0)
                {
                    // special case of PES2009 settings.exe (full game)
                    fseek(f, -0x40, SEEK_CUR);
                    fwrite("\xc3\xcc\xcc\xcc\xcc\xcc\xcc\xcc",8,1,f);
                    fwrite("\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc",8,1,f);
                    fwrite("\xc3\xcc\xcc\xcc\xcc\xcc\xcc\xcc",8,1,f);
                    fwrite("\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc",8,1,f);
                }
                else if (memcmp(buf, zero, 0x20)!=0) {
					fseek(f, -0x20, SEEK_CUR);
					fwrite(zero, 0x20, 1, f);
				}
			}
	
			if (SeekEntryPoint(f))
			{
				// write new entry point
				fwrite(&newEntryPoint, sizeof(DWORD), 1, f);
				//wprintf("New entry point: %08x\n", newEntryPoint);
			}
	        /*
			if (SeekCodeSectionFlags(f))
			{
				DWORD flags;
				fread(&flags, sizeof(DWORD), 1, f);
				if (flags & 0x20000000) {
					// if section was marked as executable (meaning that it's
					// a normal EXE - not compressed one), then turn off the
					// writeable flag that we turned on when installing
					flags &= 0x7fffffff;
				}
				fseek(f, -sizeof(DWORD), SEEK_CUR);
				fwrite(&flags, sizeof(DWORD), 1, f);
			}
	        */
			fclose(f);
	
			// show message box with error msg
			wchar_t buf[BUFLEN];
			ZeroMemory(buf, WBUFLEN);
			swprintf(buf, L"\
======== SUCCESS! =========\n\
Setup has removed Kitserver from\n\
%s.", fileName);
	
			wcscat(outmsg, buf);
			continue;
		}
		else
		{
			// show message box with error msg
			wchar_t buf[BUFLEN];
			ZeroMemory(buf, WBUFLEN);
			swprintf(buf, L"\
======== ERROR! =========\n\
Setup failed to remove Kitserver from\n\
%s.\n\
\n\
(No modifications made.)\n\
Verify that the executable is not\n\
READ-ONLY, and try again.", fileName);
	
			wcscat(outmsg, buf);
            result = false;
			continue;
		}
	}
	
	UpdateInfo();
	
	if (!quiet) MessageBox(hWnd, outmsg, lang("MsgTitle"), 0);	
    outs += outmsg;
    return result;
}

/**
 * Updates the information about exe file.
 */
void UpdateInfo(void)
{
	if (g_noFiles) return;

	EnableWindow(g_installButtonControl, FALSE);
	EnableWindow(g_removeButtonControl, FALSE);
	
	bool isValidExe[2] = {false, false};
	bool isInstalled[2] = {false, false};
	
	for (int i=0; i<2; i++) {
		HWND listControl = (i==0)?g_exeListControl:g_setListControl;
		HWND infoControl = (i==0)?g_exeInfoControl:g_setInfoControl;
		
		wchar_t fileName[BUFLEN];
		ZeroMemory(fileName, WBUFLEN);
		wcscpy(fileName, L"..\\");
		wchar_t* p = fileName + wcslen(fileName);
	
		// get currently selected item and its text
		int idx = (int)SendMessage(listControl, CB_GETCURSEL, 0, 0);
		if (idx == 0) {
			SendMessage(infoControl, WM_SETTEXT, (WPARAM)0, 
					(LPARAM)L"\0");
			continue;
		}
		SendMessage(listControl, CB_GETLBTEXT, idx, (LPARAM)p);
	
		// check if it's a recognizable EXE-file
		if (GetRealGameVersion(fileName) == -1)
		{
			SendMessage(infoControl, WM_SETTEXT, (WPARAM)0, 
					(LPARAM)L"Unrecognized EXE-file: kitserver cannot work with it.\n\0");
			continue;
		}
		
		if (isRealGame(GetRealGameVersion(fileName)) != (i==0)) {
			if (i==0) {
				SendMessage(infoControl, WM_SETTEXT, (WPARAM)0, 
					(LPARAM)L"You selected a not supported game version.\0");
			} else {
                wchar_t b[1024];
                ZeroMemory(b,sizeof(b));
                swprintf(b,L"You selected a game executable instead a a settings  executable (type=%d)",GetRealGameVersion(fileName));

				SendMessage(infoControl, WM_SETTEXT, (WPARAM)0, (LPARAM)b);
			}
			continue;
		}
		
		isValidExe[i] = true;
	
		FILE* f = _wfopen(fileName, L"rb");
		if (f != NULL)
		{
			IMAGE_SECTION_HEADER dataHeader;
			ZeroMemory(&dataHeader, sizeof(IMAGE_SECTION_HEADER));
			DWORD dataOffset = 0;
			DWORD dataVA = 0;
	
			// find empty space at the end of .rdata
			bool rdataFound(false);
            if (GetGameVersion(GetRealGameVersion(fileName)) >= gvPES2013demo) {
                rdataFound = SeekSectionHeader(f,".data");
                MyMessageBox(L"it is gvPES2013! (%d)", 
                        GetRealGameVersion(fileName));
            } 
            else {
                rdataFound = SeekSectionHeader(f, ".rdata");
                MyMessageBox(L"it is NOT gvPES2013... (%d)", 
                        GetRealGameVersion(fileName));
            }
            if (!rdataFound) rdataFound = SeekSectionHeader(f, ".rsrc");
            if (!rdataFound) rdataFound = SeekSectionHeader(f, ".IN");
			if (rdataFound) {
				fread(&dataHeader, sizeof(IMAGE_SECTION_HEADER), 1, f);
	
				dataVA = dataHeader.VirtualAddress + dataHeader.SizeOfRawData;
				if (dataHeader.PointerToRawData != 0)
					dataOffset = dataHeader.PointerToRawData + dataHeader.SizeOfRawData;
				else
					dataOffset = dataHeader.VirtualAddress + dataHeader.SizeOfRawData;
	
				// shift 32 bytes back
				dataOffset -= 0x20;
				dataVA -= 0x20;
			}
	
			MyMessageBox(L"dataOffset = %08x", dataOffset);
			MyMessageBox(L"dataVA = %08x", dataVA);
	
			if (dataOffset != 0) {
				// if installed this should have some data
				BYTE zero[0x1c];
				ZeroMemory(zero, 0x1c);
				fseek(f, dataOffset, SEEK_SET);
	
				// read saved old entry point
				DWORD savedEntryPoint = 0;
				fread(&savedEntryPoint, sizeof(DWORD), 1, f);
                size_t readSize = 0x18;
                if (savedEntryPoint == 0) {
                    // try 0x1c
                    dataOffset += 4;
                    dataVA += 4;
                    fseek(f, dataOffset, SEEK_SET);
                    fread(&savedEntryPoint, sizeof(DWORD), 1, f);
                    readSize -= 4;
                }
	
				// read kitserver DLL name
				char buf[0x18];
				ZeroMemory(buf, 0x18);
				fseek(f, sizeof(DWORD), SEEK_CUR);
				fread(buf, readSize, 1, f);
				char* dllFilename = strrchr(buf,'\\');
                dllFilename = (dllFilename)?dllFilename+1:buf;
				isInstalled[i] = false;
				
				if (savedEntryPoint != 0 &&
						strcmp(dllFilename, (i==0)?DLL_NAME:DLL_NAME_SET) == 0) {
					isInstalled[i] = true;
				}
				
				if (isInstalled[i])
				{
					char exeFolderName[BUFLEN];
					ZeroMemory(exeFolderName, BUFLEN);
					strncpy(exeFolderName,buf,
                        max(0,dllFilename-buf-1));
	
					if (strcmp(exeFolderName, patchFolderName) == 0) {
						SendMessage(infoControl, WM_SETTEXT, (WPARAM)0, 
								(LPARAM)L"Kitserver ATTACHED to this exe.\0");
								
						EnableWindow(g_installButtonControl, FALSE);
						EnableWindow(g_removeButtonControl, TRUE);
					}
					else
					{
						wchar_t temp[BUFLEN];
						ZeroMemory(temp,BUFLEN*sizeof(wchar_t));
						wchar_t* temp2 = Utf8::ansiToUnicode(exeFolderName);
						swprintf(temp, 
                            L"WARN: Kitserver from folder \"%s\" ATTACHED "
                            L"to this exe.\0", temp2);
						Utf8::free(temp2);
						
						SendMessage(infoControl, WM_SETTEXT, (WPARAM)0, 
								(LPARAM)temp);
						EnableWindow(g_installButtonControl, FALSE);
						EnableWindow(g_removeButtonControl, TRUE);
					};
				}
				else
				{
					SendMessage(infoControl, WM_SETTEXT, (WPARAM)0, 
							(LPARAM)L"Kitserver not attached.\0");
					EnableWindow(g_installButtonControl, TRUE);
					EnableWindow(g_removeButtonControl, FALSE);
	
				}
			}
			else
			{
				SendMessage(infoControl, WM_SETTEXT, (WPARAM)0,
						(LPARAM)L"Information unavailable\0");
			}
			fclose(f);
			}
			else
			{
				SendMessage(infoControl, WM_SETTEXT, (WPARAM)0, 
						(LPARAM)L"[ERROR: Can't open file.]\0");
			}
	}
	
	if (isValidExe[0] || isValidExe[1]) {
		if (isInstalled[0] || isInstalled[1])
			EnableWindow(g_removeButtonControl, true);
		
		if (!isInstalled[0] || !isInstalled[1])
			EnableWindow(g_installButtonControl, true);
		
	}
}

/**
 * Initializes all controls
 */
void InitControls(void)
{
	// Build the drop-down list
	WIN32_FIND_DATA fData;
	wchar_t pattern[BUFLEN];
	ZeroMemory(pattern, WBUFLEN);

	wcscpy(pattern, L"..\\*.exe");

    int count = 1, selectedIndex = 0;
    int count1 = 1, selectedIndex1 = 0;
	HANDLE hff = FindFirstFile(pattern, &fData);
	if (hff == INVALID_HANDLE_VALUE) 
	{
		// none found.
		g_noFiles = true;
		return;
	}
	
	SendMessage(g_exeListControl, CB_ADDSTRING, (WPARAM)0, (LPARAM)lang("NoAction"));
	SendMessage(g_exeListControl, WM_SETTEXT, (WPARAM)0, (LPARAM)lang("NoAction"));
	SendMessage(g_setListControl, CB_ADDSTRING, (WPARAM)0, (LPARAM)lang("NoAction"));
	SendMessage(g_setListControl, WM_SETTEXT, (WPARAM)0, (LPARAM)lang("NoAction"));
	
	while(true)
	{
		if (wcsicmp(fData.cFileName, L"settings.exe") != 0) // skip settings.exe
		{
			SendMessage(g_exeListControl, CB_ADDSTRING, (WPARAM)0, (LPARAM)fData.cFileName);
			SendMessage(g_exeListControl, WM_SETTEXT, (WPARAM)0, (LPARAM)fData.cFileName);
			count++; //only count the listed exe files
		}
		
		if (wcsnicmp(fData.cFileName, patchExeName, wcslen(patchExeName)) != 0) // skip default game exe
		{
			SendMessage(g_setListControl, CB_ADDSTRING, (WPARAM)0, (LPARAM)fData.cFileName);
			SendMessage(g_setListControl, WM_SETTEXT, (WPARAM)0, (LPARAM)fData.cFileName);
			count1++;
		}
		
        if (wcsnicmp(fData.cFileName, patchExeName, wcslen(patchExeName)) == 0) { // auto-select
            selectedIndex = count - 1;
        }
        
        if (wcsicmp(fData.cFileName, L"settings.exe") == 0) { // auto-select settings.exe
            selectedIndex1 = count1 - 1;
        }

		// proceed to next file
		if (!FindNextFile(hff, &fData)) break;
	}
	FindClose(hff);

	SendMessage(g_exeListControl, CB_SETCURSEL, (WPARAM)selectedIndex, (LPARAM)0);
	EnableWindow(g_exeListControl, TRUE);

	SendMessage(g_setListControl, CB_SETCURSEL, (WPARAM)selectedIndex1, (LPARAM)0);
	EnableWindow(g_setListControl, TRUE);

}

void ShowHelpText()
{
	if (MessageBox(hWnd, lang("HelpText"), lang("HelpTextTitle"), MB_YESNO) == IDYES) {
		ShellExecute(hWnd, L"open", lang("ManualPath"), NULL, NULL, SW_SHOWNORMAL);
	}
	
	return;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_DESTROY:
			// Exit the application when the window closes
			PostQuitMessage(1);
			return true;

		case WM_COMMAND:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				if ((HWND)lParam == g_installButtonControl)
				{
					InstallKserv();
				}
				else if ((HWND)lParam == g_removeButtonControl)
				{
					RemoveKserv();
				}
				else if ((HWND)lParam == g_helpButtonControl)
				{
					ShowHelpText();
				}
			}
			else if (HIWORD(wParam) == CBN_SELCHANGE)
			{
				if ((HWND)lParam == g_exeListControl || (HWND)lParam == g_setListControl)
				{
					UpdateInfo();
				}
			}
			break;
	}
	return DefWindowProc(hwnd,uMsg,wParam,lParam);
}

bool InitApp(HINSTANCE hInstance, LPSTR lpCmdLine)
{
	WNDCLASSEX wcx;

	// cbSize - the size of the structure.
	wcx.cbSize = sizeof(WNDCLASSEX);
	wcx.style = CS_HREDRAW | CS_VREDRAW;
	wcx.lpfnWndProc = (WNDPROC)WindowProc;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = hInstance;
	wcx.hIcon = LoadIcon(NULL,IDI_APPLICATION);
	wcx.hCursor = LoadCursor(NULL,IDC_ARROW);
	wcx.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wcx.lpszMenuName = NULL;
	wcx.lpszClassName = L"SETUPCLS";
	wcx.hIconSm = NULL;

	// Register the class with Windows
	if(!RegisterClassEx(&wcx))
		return false;

	return true;
}

HWND BuildWindow(int nCmdShow)
{
	DWORD style, xstyle;
	HWND retval;

	style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	xstyle = WS_EX_LEFT;

	retval = CreateWindowEx(xstyle,
        L"SETUPCLS",      // class name
        patchTitle, // title for our window (appears in the titlebar)
        style,
        CW_USEDEFAULT,  // initial x coordinate
        CW_USEDEFAULT,  // initial y coordinate
        WIN_WIDTH, WIN_HEIGHT,   // width and height of the window
        NULL,           // no parent window.
        NULL,           // no menu
        NULL,           // no creator
        NULL);          // no extra data

	if (retval == NULL) return NULL;  // BAD.

	ShowWindow(retval,nCmdShow);  // Show the window
	return retval; // return its handle for future use.
}

void ReadPatchInfo()
{
	wchar_t patchFile[BUFLEN];
	swprintf(patchFile, L"%s\\patch.cfg", mydir);
	
	FILE* cfg = _wfopen(patchFile, L"rt");
	if (cfg == NULL) return;
	
	DWORD firstDWORD;
	bool unicodeFile = false;
	fgets((char*)&firstDWORD, 4, cfg);
	if ((firstDWORD & 0xffffff) == 0xbfbbef) {
		// UTF8
		fseek(cfg, 3, SEEK_SET);
	} else if ((firstDWORD & 0xffff) == 0xfeff) {
		// Unicode Little Endian
		unicodeFile = true;
		fseek(cfg, 2, SEEK_SET);
	} else {
		// no supported BOM detected, asume UTF8
		fseek(cfg, 0, SEEK_SET);
	}

	char a_str[BUFLEN];
	wchar_t str[BUFLEN];
	wchar_t name[BUFLEN];
	int value = 0;

	wchar_t *pName = NULL, *pValue = NULL, *comment = NULL;
	while (!feof(cfg))
	{
		if (!unicodeFile) {
			ZeroMemory(str, WBUFLEN);
			ZeroMemory(a_str, BUFLEN);
			fgets(a_str, BUFLEN-1, cfg);
			Utf8::fUtf8ToUnicode(str, a_str);
		} else {
			ZeroMemory(str, WBUFLEN);
			fgetws(str, BUFLEN-1, cfg);
		}

		// skip comments
		comment = wcsstr(str, L"#");
		if (comment != NULL) comment[0] = '\0';

		// parse the line
		pName = pValue = NULL;
		ZeroMemory(name, WBUFLEN); value = 0;
		wchar_t* eq = wcsstr(str, L"=");
		if (eq == NULL || eq[1] == '\0') continue;

		eq[0] = '\0';
		pName = str; pValue = eq + 1;

		swscanf(pName, L"%s", name);

		if (wcscmp(name, L"Title")==0)
		{
			wchar_t* startQuote = wcsstr(pValue, L"\"");
			if (startQuote == NULL) continue;
			wchar_t* endQuote = wcsstr(startQuote + 1, L"\"");
			if (endQuote == NULL) continue;

			wcscat(patchTitle, L" - \0");
            memcpy(patchTitle + wcslen(patchTitle), startQuote + 1, SW * (endQuote - startQuote - 1));
		}
		else if (wcscmp(name, L"ExeFilename")==0)
		{
			wchar_t* startQuote = wcsstr(pValue, L"\"");
			if (startQuote == NULL) continue;
			wchar_t* endQuote = wcsstr(startQuote + 1, L"\"");
			if (endQuote == NULL) continue;
				
            memcpy(patchExeName, startQuote + 1, SW * (endQuote - startQuote - 1));
		}
	}
	fclose(cfg);

	return;
}

/**
 * parse cmd-line into a map of name/value pairs
 */
void parseCmdLine(map<wstring,wstring>& params, int first=1)
{
    int argc = 0;
    wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    for (int i=first; i<argc; i++)
    {
        wstring arg(argv[i]);
        int sep = arg.find('=');
        if (sep != string::npos) 
            params.insert(pair<wstring,wstring>(arg.substr(0,sep),arg.substr(sep+1)));
        else
            params.insert(pair<wstring,wstring>(arg,L""));
    }
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	MSG msg; int retval;

 	if(InitApp(hInstance, lpCmdLine) == false)
		return 0;
		
	readConfig(L".\\config.txt");
	_getConfig("kload", "lang", DT_STRING, 1, setupConfig);
		
	readLang(setupLang, hInstance);
		
	//detect the folder setup is executed from
	wchar_t temp[BUFLEN];
	ZeroMemory(temp, WBUFLEN);
	GetModuleFileName(hInstance, temp, BUFLEN);
	wchar_t *q = temp + wcslen(temp);
	while ((q != mydir) && (*q != '\\')) { *q = '\0'; q--; }
	*q = '\0';
	wcscpy(mydir, temp);
	while ((q != temp) && (*q != '\\')) { q--; }
	Utf8::fUnicodeToAnsi(patchFolderName, q+1);
	
	sprintf(installDllPath,"%s\\%s", patchFolderName, DLL_NAME);
	sprintf(installDllSetPath,"%s\\%s", patchFolderName, DLL_NAME_SET);
	
	ZeroMemory(patchTitle, WBUFLEN);
	wcscpy(patchTitle, SETUP_WINDOW_TITLE);
	ZeroMemory(patchExeName, WBUFLEN);
	wcscpy(patchExeName, DEFAULT_EXE_NAME);
	//wcscpy(patchExeName, DEFAULT_DEMO_EXE_NAME);
	
	//Look for information of a patch
	ReadPatchInfo();

    // cmd-line interface
    if (strcmp(lpCmdLine,"")!=0)
    {
        FILE* f = fopen("manager.log","wt");
        if (f) {
            map<wstring,wstring> params;
            parseCmdLine(params);

            if (params.find(L"--install") != params.end())
            {
                // install
                wstring gfile, sfile;
                map<wstring,wstring>::iterator mit;
                mit = params.find(L"--gfile");
                if (mit != params.end()) gfile = mit->second;
                mit = params.find(L"--sfile");
                if (mit != params.end()) sfile = mit->second;

                wstring outs;
                bool result = InstallKserv(gfile,sfile,outs);
                fwprintf(f,L"%s\n",outs.c_str());
                fclose(f);

                exit(result?0:1);
            }
            else if (params.find(L"--remove") != params.end())
            {
                // remove
                wstring gfile, sfile;
                map<wstring,wstring>::iterator mit;
                mit = params.find(L"--gfile");
                if (mit != params.end()) gfile = mit->second;
                mit = params.find(L"--sfile");
                if (mit != params.end()) sfile = mit->second;

                wstring outs;
                bool result = RemoveKserv(gfile,sfile,outs);
                fwprintf(f,L"%s\n",outs.c_str());
                fclose(f);

                exit(result?0:1);
            }
            else if (params.find(L"--help") != params.end())
            {
                // print out the usage
                fwprintf(f,L"Setup can be run as follows:\n\n");
                fwprintf(f,L"setup --install --gfile=<game-exe> --sfile=<settings-exe>\n");
                fwprintf(f,L"setup --remove --gfile=<game-exe> --sfile=<settings-exe>\n\n");
                fwprintf(f,L"Example: setup --remove --gfile=..\\pes2009.exe --sfile=..\\settings.exe\n");
                exit(0);
            }
        }
        return 1;
    }

	hWnd = BuildWindow(nCmdShow);
	if(hWnd == NULL)
		return 0;

	// build GUI
	if (!BuildControls(hWnd))
		return 0;

	// Initialize all controls
	InitControls();
	UpdateInfo();

	// show credits
	wchar_t buf[BUFLEN];
	ZeroMemory(buf, WBUFLEN);
	wcsncpy(buf, CREDITS, BUFLEN-1);
	SendMessage(g_statusTextControl, WM_SETTEXT, 0, (LPARAM)buf);

	while((retval = GetMessage(&msg,NULL,0,0)) != 0)
	{
		if(retval == -1)
			return 0;	// an error occured while getting a message

		if (!IsDialogMessage(hWnd, &msg)) // need to call this to make WS_TABSTOP work
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return 0;
}
