#include "pch.h"
#include "IconImpl.h"
#include <shobjidl.h>

#pragma comment(lib, "shell32.lib")

// Get Explorer-style image (thumbnail first, then icon fallback)
HRESULT GetFileIconImpl(LPCWSTR filePath, UINT size, HBITMAP* phBitmap)
{
    if (!filePath || !phBitmap)
        return E_INVALIDARG;
    
    *phBitmap = nullptr;

    IShellItemImageFactory* sif = nullptr;
    HRESULT hr = SHCreateItemFromParsingName(filePath, nullptr, IID_IShellItemImageFactory, reinterpret_cast<void**>(&sif));
    if (FAILED(hr))
        return hr;

    SIZE sz = { (LONG)size, (LONG)size };
    HBITMAP hbm = nullptr;

    // 1) サムネイルを要求（ここで PNG なら画像の縮小版が返る）
    DWORD flags = SIIGBF_RESIZETOFIT | SIIGBF_BIGGERSIZEOK | SIIGBF_THUMBNAILONLY;
    hr = sif->GetImage(sz, flags, &hbm);
    
    if (SUCCEEDED(hr) && hbm)
    {
        printf("GetFileIcon: Got thumbnail\n");
        sif->Release();
        *phBitmap = hbm;
        return S_OK;
    }

    // 2) サムネイル無理ならアイコンで
    flags = SIIGBF_RESIZETOFIT | SIIGBF_BIGGERSIZEOK | SIIGBF_ICONONLY;
    hr = sif->GetImage(sz, flags, &hbm);
    
    if (SUCCEEDED(hr) && hbm)
    {
        printf("GetFileIcon: Got icon (thumbnail not available)\n");
        sif->Release();
        *phBitmap = hbm;
        return S_OK;
    }

    sif->Release();
    return hr;
}

