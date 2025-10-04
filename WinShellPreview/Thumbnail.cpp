#include "pch.h"
#include "ThumbnailImpl.h"
#include "BitmapUtils.h"
#include "PreviewHandler.h"
#include <memory>
#include <gdiplus.h>
#include <algorithm>
#include <shobjidl.h>
#include <propvarutil.h>
#include <propkey.h>

#pragma comment(lib, "propsys.lib")

using namespace Gdiplus;

// Get original media dimensions from Shell property store
HRESULT GetMediaDimensions(LPCWSTR filePath, UINT* width, UINT* height)
{
    *width = *height = 0;

    IShellItem2* psi = nullptr;
    HRESULT hr = SHCreateItemFromParsingName(filePath, nullptr, IID_IShellItem2, reinterpret_cast<void**>(&psi));
    if (FAILED(hr))
        return hr;

    // Windows Shell のプロパティストアを取得
    IPropertyStore* pStore = nullptr;
    hr = psi->GetPropertyStore(GPS_DEFAULT, IID_IPropertyStore, reinterpret_cast<void**>(&pStore));
    if (FAILED(hr)) {
        psi->Release();
        return hr;
    }

    PROPVARIANT varW = {}, varH = {};

    // 1) 画像・動画系ファイルの「幅」「高さ」プロパティを取得
    hr = pStore->GetValue(PKEY_Image_HorizontalSize, &varW);
    if (SUCCEEDED(hr)) {
        pStore->GetValue(PKEY_Image_VerticalSize, &varH);
    } else {
        // 2) 代替キー（例：ビデオの場合）
        pStore->GetValue(PKEY_Video_FrameWidth, &varW);
        pStore->GetValue(PKEY_Video_FrameHeight, &varH);
    }

    // 値が有効なら代入
    if (SUCCEEDED(hr) && varW.vt == VT_UI4 && varH.vt == VT_UI4) {
        *width  = varW.ulVal;
        *height = varH.ulVal;
        PropVariantClear(&varW);
        PropVariantClear(&varH);
        pStore->Release();
        psi->Release();
        return S_OK;
    }

    PropVariantClear(&varW);
    PropVariantClear(&varH);
    pStore->Release();
    psi->Release();
    return E_FAIL;
}

HRESULT GetFileThumbnailImpl(LPCWSTR filePath, UINT size, HBITMAP* phBitmap)
{
    if (!filePath || !phBitmap)
        return E_INVALIDARG;

    *phBitmap = nullptr;

    PreviewHandler handler;
    WTS_ALPHATYPE alphaType;
    HBITMAP hRawBitmap = nullptr;
    HRESULT hr = handler.GetThumbnail(filePath, size, &hRawBitmap, &alphaType);
    
    if (FAILED(hr) || !hRawBitmap)
        return hr;
    
    // Get original size
    BITMAP bmpInfo = {};
    GetObject(hRawBitmap, sizeof(BITMAP), &bmpInfo);
    char debugMsg[256];
    sprintf_s(debugMsg, "Original bitmap size: %dx%d\n", bmpInfo.bmWidth, bmpInfo.bmHeight);
    printf("%s", debugMsg);
    
    // Check alpha type
    const char* alphaTypeStr = "UNKNOWN";
    if (alphaType == WTSAT_ARGB) alphaTypeStr = "WTSAT_ARGB (transparent)";
    else if (alphaType == WTSAT_RGB) alphaTypeStr = "WTSAT_RGB (opaque)";
    else if (alphaType == WTSAT_UNKNOWN) alphaTypeStr = "WTSAT_UNKNOWN";
    
    sprintf_s(debugMsg, "WTS_ALPHATYPE: %s (%d)\n", alphaTypeStr, alphaType);
    printf("%s", debugMsg);
    
    // Get original media dimensions
    UINT origWidth = 0, origHeight = 0;
    hr = GetMediaDimensions(filePath, &origWidth, &origHeight);
    
    if (SUCCEEDED(hr) && origWidth > 0 && origHeight > 0)
    {
        sprintf_s(debugMsg, "Original media dimensions: %dx%d\n", origWidth, origHeight);
        printf("%s", debugMsg);
        
        // Calculate aspect ratio and determine crop size
        float aspectRatio = (float)origWidth / (float)origHeight;
        int cropWidth = size;
        int cropHeight = size;
        
        if (aspectRatio > 1.0f) {
            // Horizontal image - crop from top
            cropHeight = (int)(size / aspectRatio);
        } else if (aspectRatio < 1.0f) {
            // Vertical image - crop from left
            cropWidth = (int)(size * aspectRatio);
        }
        
        sprintf_s(debugMsg, "Calculated crop size: %dx%d (aspect ratio: %.2f)\n", cropWidth, cropHeight, aspectRatio);
        printf("%s", debugMsg);
        
        // Crop using calculated dimensions
        HBITMAP hCropped = CropFromTopLeft(hRawBitmap, cropWidth, cropHeight);
        
        if (hCropped)
        {
            BITMAP cropInfo = {};
            GetObject(hCropped, sizeof(BITMAP), &cropInfo);
            sprintf_s(debugMsg, "Cropped bitmap size: %dx%d\n", cropInfo.bmWidth, cropInfo.bmHeight);
            printf("%s", debugMsg);
            
            DeleteObject(hRawBitmap);
            *phBitmap = hCropped;
            return S_OK;
        }
    }
    
    // Fallback: use original bitmap if dimension detection failed
    printf("Using original bitmap (dimension detection failed or crop failed)\n");
    *phBitmap = hRawBitmap;
    return S_OK;
}

