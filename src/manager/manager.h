#ifndef _MANAGER_MAIN_
#define _MANAGER_MAIN_

#include <kconf.h>

#ifdef MYDLL_RELEASE_BUILD
#define SETUP_WINDOW_TITLE L"Kitserver EXE Manager"
#else
#define SETUP_WINDOW_TITLE L"Kitserver EXE Manager (debug build)"
#endif
#define CREDITS L"About: " KITSERVER_VERSION_STRING " by Juce and Robbie."

#endif

