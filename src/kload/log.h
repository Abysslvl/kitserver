#ifndef _LOG_
#define _LOG_

#include "manage.h"

#define KEXPORT EXTERN_C __declspec(dllexport)

#ifndef THISMOD
#define THISMOD NULL
#endif

#define LOG(...) _LogX(THISMOD, __VA_ARGS__)

#ifndef MYDLL_RELEASE_BUILD
#define TRACE(...) _LogX(THISMOD, __VA_ARGS__)
#else
#define TRACE(...)
#endif

KEXPORT void OpenLog(const wchar_t* logName);
KEXPORT void CloseLog();
KEXPORT void _LogX(KMOD *caller, const wchar_t* format, ...);

#endif
