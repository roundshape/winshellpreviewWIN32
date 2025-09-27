#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <thumbcache.h>
#include <wincodec.h>
#include <wincodecsdk.h>
#include <combaseapi.h>
#include <wrl/client.h>
#include <propkey.h>
#include <propvarutil.h>
#include <objbase.h>
#include <shobjidl_core.h>
#include <shobjidl.h>
#include <shlguid.h>
#include <gdiplus.h>
#include <memory>
#include <string>
#include <comdef.h>
#include <commoncontrols.h>


#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "gdiplus.lib")