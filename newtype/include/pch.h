#pragma once

#if !defined( _DEBUG )
# define _CRT_SECURE_NO_WARNINGS
# define _SCL_SECURE_NO_WARNINGS
#endif

#define NTDDI_VERSION NTDDI_WIN10
#define _WIN32_WINNT _WIN32_WINNT_WIN10

#include <sdkddkver.h>

#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <commctrl.h>
#include <richedit.h>
#include <ctime>
//#include <psapi.h>
//#include <dbghelp.h>
#include <avrt.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

// VC++ Runtime Headers
#ifdef _DEBUG
//# define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <cstdio>
#include <malloc.h>
#include <memory.h>
#include <cwchar>
#define _USE_MATH_DEFINES
#include <cmath>
#include <eh.h>
#include <intrin.h>
#include <cassert>

#include "newtype_types.h"