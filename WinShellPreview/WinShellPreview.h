#pragma once

#ifdef WINSHELLPREVIEW_EXPORTS
#define WINSHELLPREVIEW_API __declspec(dllexport)
#else
#define WINSHELLPREVIEW_API __declspec(dllimport)
#endif

extern "C" {
    WINSHELLPREVIEW_API HRESULT GetFileThumbnail(LPCWSTR filePath, UINT size, HBITMAP* phBitmap);
    WINSHELLPREVIEW_API HRESULT GetFilePreview(LPCWSTR filePath, UINT width, UINT height, HBITMAP* phBitmap);
    WINSHELLPREVIEW_API HRESULT GetFileIcon(LPCWSTR filePath, UINT size, HBITMAP* phBitmap);
    WINSHELLPREVIEW_API HRESULT SaveBitmapToFile(HBITMAP hBitmap, LPCWSTR outputPath);
    WINSHELLPREVIEW_API void ReleasePreviewBitmap(HBITMAP hBitmap);
}