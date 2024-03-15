// log.cpp
#define UNICODE
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include "shared.h"
#include "log.h"
#include "utf8.h"

static HANDLE mylog = INVALID_HANDLE_VALUE;

//////////////////////////////////////////////////
// FUNCTIONS
//////////////////////////////////////////////////


// Creates log file
void OpenLog(const wchar_t* logName)
{
    mylog = CreateFile(
            logName,                          // file to create 
            GENERIC_WRITE,                    // open for writing 
            FILE_SHARE_READ | FILE_SHARE_DELETE,   // do not share 
            NULL,                             // default security 
            CREATE_ALWAYS,                    // overwrite existing 
            FILE_ATTRIBUTE_NORMAL,            // normal file 
            NULL);                            // no attr. template 

    if (mylog == INVALID_HANDLE_VALUE) return;
    DWORD wbytes;
    BYTE buf[3] = {0xef, 0xbb, 0xbf};		// UTF8
    WriteFile(mylog, (LPVOID)buf, 3,(LPDWORD)&wbytes, NULL);
}

// Closes log file
void CloseLog()
{
	if (mylog != INVALID_HANDLE_VALUE) CloseHandle(mylog);
	mylog = INVALID_HANDLE_VALUE;
}

// Simple logger
void _Log(KMOD *caller, const wchar_t *msg)
{
	if (!caller || mylog == INVALID_HANDLE_VALUE) return;
	/*#ifdef MYDLL_RELEASE_BUILD
	if (caller->debug < 1) return;
	#endif*/
	
	DWORD wbytes;
	wchar_t buf[BUFLEN];
	ZeroMemory(buf, WBUFLEN);
	
	if (mylog != INVALID_HANDLE_VALUE) 
	{
		swprintf(buf, L"{%s} %s\r\n", caller->nameShort, msg);
		BYTE* utf8Buf = Utf8::unicodeToUtf8(buf);
		WriteFile(
                mylog,(LPVOID)utf8Buf, 
                Utf8::byteLength(utf8Buf),(LPDWORD)&wbytes, NULL);
		Utf8::free(utf8Buf);
	}
}

// Universal logger
KEXPORT void _LogX(KMOD *caller, const wchar_t *format, ...)
{
    if (mylog == INVALID_HANDLE_VALUE)
        return;

    wchar_t buffer[BUFLEN];
    memset(buffer,0,sizeof(buffer));

    va_list params;
    va_start(params, format);
    _vsnwprintf(buffer, 512, format, params);
    va_end(params);

    _Log(caller,buffer);
}

