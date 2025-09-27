#include "pch.h"
#include <gdiplus.h>

using namespace Gdiplus;

static ULONG_PTR g_gdiplusToken = 0;

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        // GDI+を初期化
        {
            GdiplusStartupInput gdiplusStartupInput;
            if (GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, nullptr) != Ok)
            {
                return FALSE;
            }
        }
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        // GDI+を終了
        if (g_gdiplusToken != 0)
        {
            GdiplusShutdown(g_gdiplusToken);
            g_gdiplusToken = 0;
        }
        break;
    }
    return TRUE;
}