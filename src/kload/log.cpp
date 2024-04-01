// log.cpp
#define UNICODE
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include "shared.h"
#include "log.h"
#include "utf8.h"

FILE* mylog;

// Create log file
void OpenLog(const wchar_t* logName)
{
    if (mylog == nullptr)
        mylog = _wfopen(logName, L"w");
}

// Close log file
void CloseLog()
{
	if (mylog != nullptr)
		fclose(mylog);
}

// Simple logger
void _Log(KMOD *caller, const wchar_t *msg)
{
	if (!caller || mylog == nullptr)
		return;
	
	DWORD wbytes;
	wchar_t buf[MAX_PATH];
	swprintf(buf, L"{%s} %s\r\n", caller->nameShort, msg);
	BYTE* encode = Utf8::unicodeToUtf8(buf);
	fwrite(encode, strlen((char*)encode), 1, mylog);
	fflush(mylog);
	Utf8::free(encode);
}

// Universal logger
KEXPORT void _LogX(KMOD *caller, const wchar_t *format, ...)
{
    if (mylog == nullptr)
        return;

    wchar_t buffer[MAX_PATH];
    va_list params;
    va_start(params, format);
    vswprintf(buffer, MAX_PATH, format, params);
    va_end(params);

    _Log(caller,buffer);
}

