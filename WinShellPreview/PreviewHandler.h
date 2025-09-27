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

private:
    HRESULT GetThumbnailUsingIThumbnailProvider(LPCWSTR pszFilePath, UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha);
    HRESULT GetThumbnailUsingIExtractImage(LPCWSTR pszFilePath, UINT cx, UINT cy, HBITMAP* phbmp);
    HRESULT GetThumbnailUsingIThumbnailCache(LPCWSTR pszFilePath, UINT cx, HBITMAP* phbmp);
};