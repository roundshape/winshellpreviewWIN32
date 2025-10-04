#include "pch.h"
#include "WinShellPreview.h"
#include "ThumbnailImpl.h"
#include "PreviewImpl.h"
#include "IconImpl.h"
#include "BitmapUtils.h"

extern "C" {

WINSHELLPREVIEW_API HRESULT GetFileThumbnail(LPCWSTR filePath, UINT size, HBITMAP* phBitmap)
{
    return GetFileThumbnailImpl(filePath, size, phBitmap);
}

WINSHELLPREVIEW_API HRESULT GetFilePreview(LPCWSTR filePath, UINT width, UINT height, HBITMAP* phBitmap)
{
    return GetFilePreviewImpl(filePath, width, height, phBitmap);
}

WINSHELLPREVIEW_API HRESULT GetFileIcon(LPCWSTR filePath, UINT size, HBITMAP* phBitmap)
{
    return GetFileIconImpl(filePath, size, phBitmap);
}

WINSHELLPREVIEW_API HRESULT SaveBitmapToFile(HBITMAP hBitmap, LPCWSTR outputPath)
{
    return SaveBitmapToFileImpl(hBitmap, outputPath);
}

WINSHELLPREVIEW_API void ReleasePreviewBitmap(HBITMAP hBitmap)
{
    if (hBitmap)
    {
        DeleteObject(hBitmap);
    }
}

}

