#include "pch.h"
#include "WinShellPreview.h"
#include "PreviewHandler.h"
#include <memory>
#include <gdiplus.h>
#include <algorithm>
#include <cctype>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "windowscodecs.lib")

using namespace Gdiplus;
using Microsoft::WRL::ComPtr;

// WIC-based PNG saving function that handles premultiplied alpha correctly
HRESULT SaveHBITMAPAsPng(HBITMAP hbmp, LPCWSTR outPath)
{
    if (!hbmp || !outPath)
        return E_INVALIDARG;

    ComPtr<IWICImagingFactory> factory;
    HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
    if (FAILED(hr)) return hr;

    // Create WIC bitmap from HBITMAP - this handles premultiplied alpha correctly
    ComPtr<IWICBitmap> srcBitmap;
    hr = factory->CreateBitmapFromHBITMAP(hbmp, nullptr, WICBitmapUsePremultipliedAlpha, &srcBitmap);
    if (FAILED(hr)) return hr;

    // Create format converter to convert PBGRA to BGRA (non-premultiplied alpha)
    ComPtr<IWICFormatConverter> converter;
    hr = factory->CreateFormatConverter(&converter);
    if (FAILED(hr)) return hr;

    hr = converter->Initialize(srcBitmap.Get(), GUID_WICPixelFormat32bppBGRA, 
                              WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
    if (FAILED(hr)) return hr;

    // Create stream for output file
    ComPtr<IWICStream> stream;
    hr = factory->CreateStream(&stream);
    if (FAILED(hr)) return hr;

    hr = stream->InitializeFromFilename(outPath, GENERIC_WRITE);
    if (FAILED(hr)) return hr;

    // Create PNG encoder
    ComPtr<IWICBitmapEncoder> encoder;
    hr = factory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder);
    if (FAILED(hr)) return hr;

    hr = encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache);
    if (FAILED(hr)) return hr;

    // Create frame
    ComPtr<IWICBitmapFrameEncode> frame;
    ComPtr<IPropertyBag2> props;
    hr = encoder->CreateNewFrame(&frame, &props);
    if (FAILED(hr)) return hr;

    hr = frame->Initialize(props.Get());
    if (FAILED(hr)) return hr;

    // Write the converted bitmap data
    hr = frame->WriteSource(converter.Get(), nullptr);
    if (FAILED(hr)) return hr;

    hr = frame->Commit();
    if (FAILED(hr)) return hr;

    hr = encoder->Commit();
    return hr;
}

extern "C" {

WINSHELLPREVIEW_API HRESULT GetFileThumbnail(LPCWSTR filePath, UINT size, HBITMAP* phBitmap)
{
    if (!filePath || !phBitmap)
        return E_INVALIDARG;

    *phBitmap = nullptr;

    PreviewHandler handler;
    WTS_ALPHATYPE alphaType;
    return handler.GetThumbnail(filePath, size, phBitmap, &alphaType);
}

WINSHELLPREVIEW_API HRESULT GetFilePreview(LPCWSTR filePath, UINT width, UINT height, HBITMAP* phBitmap)
{
    if (!filePath || !phBitmap)
        return E_INVALIDARG;

    *phBitmap = nullptr;

    PreviewHandler handler;
    return handler.GetPreviewBitmap(filePath, width, height, phBitmap);
}

HBITMAP ConvertToCompatibleBitmap(HBITMAP hSourceBitmap, int width, int height)
{
    HDC hdc = GetDC(nullptr);
    HDC sourceDC = CreateCompatibleDC(hdc);
    HDC destDC = CreateCompatibleDC(hdc);

    HBITMAP hDestBitmap = CreateCompatibleBitmap(hdc, width, height);

    HBITMAP oldSourceBitmap = (HBITMAP)SelectObject(sourceDC, hSourceBitmap);
    HBITMAP oldDestBitmap = (HBITMAP)SelectObject(destDC, hDestBitmap);

    // Simply copy the bitmap
    BitBlt(destDC, 0, 0, width, height, sourceDC, 0, 0, SRCCOPY);

    SelectObject(sourceDC, oldSourceBitmap);
    SelectObject(destDC, oldDestBitmap);
    DeleteDC(sourceDC);
    DeleteDC(destDC);
    ReleaseDC(nullptr, hdc);

    return hDestBitmap;
}

HRESULT SaveBitmapAsBMP(HBITMAP hBitmap, LPCWSTR outputPath)
{
    if (!hBitmap || !outputPath)
        return E_INVALIDARG;

    BITMAP bmp = {};
    HBITMAP hCompatibleBitmap = nullptr;
    HDC hdcScreen = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    
    // Try to get bitmap info first
    if (GetObject(hBitmap, sizeof(BITMAP), &bmp) && bmp.bmWidth > 0 && bmp.bmHeight > 0)
    {
        // Bitmap info retrieved successfully, use as-is
    }
    else
    {
        // GetObject failed or returned invalid dimensions
        // This often happens with shared bitmaps from thumbnail providers
        // Try to determine size by selecting into DC
        HBITMAP oldBmp = (HBITMAP)SelectObject(hdcMem, hBitmap);
        
        // Try to get dimensions through the DC
        BITMAP dcBmp = {};
        HBITMAP currentBmp = (HBITMAP)GetCurrentObject(hdcMem, OBJ_BITMAP);
        
        if (currentBmp && GetObject(currentBmp, sizeof(BITMAP), &dcBmp) && 
            dcBmp.bmWidth > 0 && dcBmp.bmHeight > 0)
        {
            bmp = dcBmp;
        }
        else
        {
            // Still no luck, use default size and create a compatible bitmap
            bmp.bmWidth = 256;
            bmp.bmHeight = 256;
            bmp.bmBitsPixel = 24;
            bmp.bmPlanes = 1;
        }
        
        SelectObject(hdcMem, oldBmp);
        
        // Create a compatible bitmap and copy the content
        hCompatibleBitmap = CreateCompatibleBitmap(hdcScreen, bmp.bmWidth, bmp.bmHeight);
        if (!hCompatibleBitmap)
        {
            DeleteDC(hdcMem);
            ReleaseDC(nullptr, hdcScreen);
            return E_FAIL;
        }
        
        // Copy the original bitmap to the compatible one
        HDC hdcSrc = CreateCompatibleDC(hdcScreen);
        HDC hdcDst = CreateCompatibleDC(hdcScreen);
        
        HBITMAP oldSrc = (HBITMAP)SelectObject(hdcSrc, hBitmap);
        HBITMAP oldDst = (HBITMAP)SelectObject(hdcDst, hCompatibleBitmap);
        
        // Fill with white background first
        RECT rect = {0, 0, bmp.bmWidth, bmp.bmHeight};
        HBRUSH whiteBrush = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(hdcDst, &rect, whiteBrush);
        DeleteObject(whiteBrush);
        
        // Copy the bitmap
        BitBlt(hdcDst, 0, 0, bmp.bmWidth, bmp.bmHeight, hdcSrc, 0, 0, SRCCOPY);
        
        SelectObject(hdcSrc, oldSrc);
        SelectObject(hdcDst, oldDst);
        DeleteDC(hdcSrc);
        DeleteDC(hdcDst);
        
        // Update bitmap info for the compatible bitmap
        if (!GetObject(hCompatibleBitmap, sizeof(BITMAP), &bmp))
        {
            DeleteObject(hCompatibleBitmap);
            DeleteDC(hdcMem);
            ReleaseDC(nullptr, hdcScreen);
            return E_FAIL;
        }
        
        hBitmap = hCompatibleBitmap;
    }

    // Now we have a valid bitmap with proper dimensions
    HDC hdc = GetDC(nullptr);
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);

    BITMAPINFOHEADER bi = {};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmp.bmWidth;
    bi.biHeight = bmp.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = ((bmp.bmWidth * 24 + 31) / 32) * 4 * bmp.bmHeight;

    DWORD dwBmpSize = bi.biSizeImage;
    HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
    if (!hDIB)
    {
        SelectObject(memDC, oldBitmap);
        DeleteDC(memDC);
        DeleteDC(hdcMem);
        ReleaseDC(nullptr, hdcScreen);
        ReleaseDC(nullptr, hdc);
        if (hCompatibleBitmap) DeleteObject(hCompatibleBitmap);
        return E_OUTOFMEMORY;
    }

    char* lpbitmap = (char*)GlobalLock(hDIB);
    int result = GetDIBits(hdc, hBitmap, 0, bmp.bmHeight, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    
    if (result == 0)
    {
        // GetDIBits failed, cleanup and return error
        GlobalUnlock(hDIB);
        GlobalFree(hDIB);
        SelectObject(memDC, oldBitmap);
        DeleteDC(memDC);
        DeleteDC(hdcMem);
        ReleaseDC(nullptr, hdcScreen);
        ReleaseDC(nullptr, hdc);
        if (hCompatibleBitmap) DeleteObject(hCompatibleBitmap);
        return E_FAIL;
    }

    HANDLE hFile = CreateFileW(outputPath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        GlobalUnlock(hDIB);
        GlobalFree(hDIB);
        SelectObject(memDC, oldBitmap);
        DeleteDC(memDC);
        DeleteDC(hdcMem);
        ReleaseDC(nullptr, hdcScreen);
        ReleaseDC(nullptr, hdc);
        if (hCompatibleBitmap) DeleteObject(hCompatibleBitmap);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    BITMAPFILEHEADER bmfHeader = {};
    bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmfHeader.bfSize = bmfHeader.bfOffBits + dwBmpSize;
    bmfHeader.bfType = 0x4D42; // "BM"

    DWORD dwBytesWritten = 0;
    WriteFile(hFile, &bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, nullptr);
    WriteFile(hFile, &bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, nullptr);
    WriteFile(hFile, lpbitmap, dwBmpSize, &dwBytesWritten, nullptr);

    CloseHandle(hFile);
    GlobalUnlock(hDIB);
    GlobalFree(hDIB);
    SelectObject(memDC, oldBitmap);
    DeleteDC(memDC);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);
    ReleaseDC(nullptr, hdc);

    // Clean up converted bitmap if we created one
    if (hCompatibleBitmap)
    {
        DeleteObject(hCompatibleBitmap);
    }

    return S_OK;
}

WINSHELLPREVIEW_API HRESULT SaveBitmapToFile(HBITMAP hBitmap, LPCWSTR outputPath)
{
    if (!hBitmap || !outputPath)
        return E_INVALIDARG;

    std::wstring path(outputPath);
    std::wstring ext;
    
    size_t dotPos = path.find_last_of(L".");
    if (dotPos != std::wstring::npos)
    {
        ext = path.substr(dotPos + 1);
        // Convert to lowercase for comparison
        std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
    }

    // Use WIC for PNG files to handle premultiplied alpha correctly
    if (ext == L"png")
    {
        return SaveHBITMAPAsPng(hBitmap, outputPath);
    }
    else
    {
        // Use traditional BMP saving for other formats
        return SaveBitmapAsBMP(hBitmap, outputPath);
    }
}

WINSHELLPREVIEW_API void ReleasePreviewBitmap(HBITMAP hBitmap)
{
    if (hBitmap)
    {
        DeleteObject(hBitmap);
    }
}

}