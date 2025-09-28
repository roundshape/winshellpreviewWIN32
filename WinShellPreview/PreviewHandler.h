#pragma once
#include "framework.h"

class PreviewHandler
{
public:
    PreviewHandler();
    ~PreviewHandler();

    HRESULT GetThumbnail(LPCWSTR pszFilePath, UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha);
    HRESULT GetPreviewBitmap(LPCWSTR pszFilePath, UINT cx, UINT cy, HBITMAP* phbmp);
    HRESULT ExtractImage(LPCWSTR pszFilePath, UINT cx, UINT cy, HBITMAP* phbmp);
    
    // IShellItemImageFactory method (recommended for modern thumbnail generation)
    HRESULT GetImageUsingIShellItemImageFactory(LPCWSTR pszFilePath, UINT cx, HBITMAP* phbmp);
    
    // IPreviewHandler method (for actual file content preview)
    HRESULT GetPreviewUsingIPreviewHandler(LPCWSTR pszFilePath, UINT cx, UINT cy, HBITMAP* phbmp);

private:
    // Helper structures for STA thread
    struct PreviewThreadData {
        LPCWSTR filePath;
        UINT width;
        UINT height;
        HBITMAP* pBitmap;
        HRESULT result;
        HANDLE completionEvent;
    };
    
    static DWORD WINAPI PreviewSTAThread(LPVOID lpParam);
    HRESULT GetPreviewHandlerFromExtension(LPCWSTR pszFilePath, IPreviewHandler** ppPreviewHandler);
    HRESULT GetPreviewInSTAThread(LPCWSTR pszFilePath, UINT cx, UINT cy, HBITMAP* phbmp);

private:
    HRESULT GetThumbnailUsingIThumbnailProvider(LPCWSTR pszFilePath, UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha);
    HRESULT GetThumbnailUsingIExtractImage(LPCWSTR pszFilePath, UINT cx, UINT cy, HBITMAP* phbmp);
    HRESULT GetThumbnailUsingIThumbnailCache(LPCWSTR pszFilePath, UINT cx, HBITMAP* phbmp);
};