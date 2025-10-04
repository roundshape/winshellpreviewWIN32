#include "pch.h"
#include "PreviewImpl.h"
#include "PreviewHandler.h"

HRESULT GetFilePreviewImpl(LPCWSTR filePath, UINT width, UINT height, HBITMAP* phBitmap)
{
    if (!filePath || !phBitmap)
        return E_INVALIDARG;

    *phBitmap = nullptr;

    PreviewHandler handler;
    return handler.GetPreviewBitmap(filePath, width, height, phBitmap);
}

