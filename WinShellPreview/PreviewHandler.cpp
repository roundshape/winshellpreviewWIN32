#include "pch.h"
#include "PreviewHandler.h"
#include <commoncontrols.h>
#include <shellapi.h>

#pragma comment(lib, "windowscodecs.lib")

PreviewHandler::PreviewHandler()
{
    // COM initialization is handled by the application
}

PreviewHandler::~PreviewHandler()
{
    // COM cleanup is handled by the application
}

HRESULT PreviewHandler::GetThumbnail(LPCWSTR pszFilePath, UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha)
{
    if (!pszFilePath || !phbmp)
        return E_INVALIDARG;

    // Try IThumbnailProvider first (most reliable for modern file types)
    HRESULT hr = GetThumbnailUsingIThumbnailProvider(pszFilePath, cx, phbmp, pdwAlpha);

    if (FAILED(hr))
    {
        // Try IExtractImage (classic method, works well with shell extensions)
        hr = GetThumbnailUsingIExtractImage(pszFilePath, cx, cx, phbmp);
        if (SUCCEEDED(hr) && pdwAlpha)
        {
            *pdwAlpha = WTSAT_UNKNOWN;
        }
    }

    // Try thumbnail cache last (can return shared bitmaps that are harder to process)
    if (FAILED(hr))
    {
        hr = GetThumbnailUsingIThumbnailCache(pszFilePath, cx, phbmp);
    }

    return hr;
}

HRESULT PreviewHandler::GetPreviewBitmap(LPCWSTR pszFilePath, UINT cx, UINT cy, HBITMAP* phbmp)
{
    if (!pszFilePath || !phbmp)
        return E_INVALIDARG;

    return ExtractImage(pszFilePath, cx, cy, phbmp);
}

HRESULT PreviewHandler::ExtractImage(LPCWSTR pszFilePath, UINT cx, UINT cy, HBITMAP* phbmp)
{
    HRESULT hr = GetThumbnailUsingIExtractImage(pszFilePath, cx, cy, phbmp);

    if (FAILED(hr))
    {
        WTS_ALPHATYPE alphaType;
        hr = GetThumbnail(pszFilePath, min(cx, cy), phbmp, &alphaType);
    }

    return hr;
}

HRESULT PreviewHandler::GetThumbnailUsingIThumbnailProvider(LPCWSTR pszFilePath, UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha)
{
    IShellItem* pShellItem = nullptr;
    HRESULT hr = SHCreateItemFromParsingName(pszFilePath, nullptr, IID_PPV_ARGS(&pShellItem));

    if (SUCCEEDED(hr))
    {
        IThumbnailProvider* pThumbProvider = nullptr;
        hr = pShellItem->BindToHandler(nullptr, BHID_ThumbnailHandler, IID_PPV_ARGS(&pThumbProvider));

        if (SUCCEEDED(hr))
        {
            hr = pThumbProvider->GetThumbnail(cx, phbmp, pdwAlpha);
            pThumbProvider->Release();
        }

        pShellItem->Release();
    }

    return hr;
}

HRESULT PreviewHandler::GetThumbnailUsingIExtractImage(LPCWSTR pszFilePath, UINT cx, UINT cy, HBITMAP* phbmp)
{
    if (!pszFilePath || !phbmp)
        return E_INVALIDARG;

    *phbmp = nullptr;

    IShellFolder* pDesktop = nullptr;
    HRESULT hr = SHGetDesktopFolder(&pDesktop);

    if (SUCCEEDED(hr))
    {
        PIDLIST_ABSOLUTE pidl = nullptr;
        hr = pDesktop->ParseDisplayName(nullptr, nullptr, const_cast<LPWSTR>(pszFilePath), nullptr, &pidl, nullptr);

        if (SUCCEEDED(hr))
        {
            IShellFolder* pFolder = nullptr;
            PCUITEMID_CHILD pidlChild = nullptr;
            hr = SHBindToParent(pidl, IID_IShellFolder, (void**)&pFolder, &pidlChild);

            if (SUCCEEDED(hr))
            {
                IExtractImage* pExtract = nullptr;
                hr = pFolder->GetUIObjectOf(nullptr, 1, &pidlChild, IID_IExtractImage, nullptr, (void**)&pExtract);

                if (SUCCEEDED(hr))
                {
                    SIZE size = { (LONG)cx, (LONG)cy };
                    DWORD dwPriority = 0;
                    DWORD dwFlags = IEIFLAG_SCREEN | IEIFLAG_OFFLINE;
                    WCHAR szPath[MAX_PATH] = { 0 };

                    hr = pExtract->GetLocation(szPath, MAX_PATH, &dwPriority, &size, 32, &dwFlags);

                    if (SUCCEEDED(hr))
                    {
                        hr = pExtract->Extract(phbmp);
                    }

                    pExtract->Release();
                }

                if (pFolder)
                    pFolder->Release();
            }

            if (pidl)
                CoTaskMemFree(pidl);
        }

        pDesktop->Release();
    }

    return hr;
}

HRESULT PreviewHandler::GetThumbnailUsingIThumbnailCache(LPCWSTR pszFilePath, UINT cx, HBITMAP* phbmp)
{
    IThumbnailCache* pThumbCache = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_LocalThumbnailCache, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pThumbCache));

    if (SUCCEEDED(hr))
    {
        IShellItem* pShellItem = nullptr;
        hr = SHCreateItemFromParsingName(pszFilePath, nullptr, IID_PPV_ARGS(&pShellItem));

        if (SUCCEEDED(hr))
        {
            ISharedBitmap* pSharedBitmap = nullptr;
            WTS_CACHEFLAGS cacheFlags;
            WTS_THUMBNAILID thumbId;

            hr = pThumbCache->GetThumbnail(pShellItem, cx, WTS_EXTRACT, &pSharedBitmap, &cacheFlags, &thumbId);

            if (SUCCEEDED(hr) && pSharedBitmap)
            {
                HBITMAP hSharedBmp = nullptr;
                hr = pSharedBitmap->GetSharedBitmap(&hSharedBmp);
                
                if (SUCCEEDED(hr) && hSharedBmp)
                {
                    // Convert shared bitmap to a regular bitmap we can work with
                    HDC hdcScreen = GetDC(nullptr);
                    HDC hdcSrc = CreateCompatibleDC(hdcScreen);
                    HDC hdcDst = CreateCompatibleDC(hdcScreen);
                    
                    // Create a compatible bitmap for the destination
                    HBITMAP hRegularBmp = CreateCompatibleBitmap(hdcScreen, cx, cx);
                    
                    if (hRegularBmp)
                    {
                        HBITMAP oldSrc = (HBITMAP)SelectObject(hdcSrc, hSharedBmp);
                        HBITMAP oldDst = (HBITMAP)SelectObject(hdcDst, hRegularBmp);
                        
                        // Fill with white background
                        RECT rect = {0, 0, (LONG)cx, (LONG)cx};
                        HBRUSH whiteBrush = CreateSolidBrush(RGB(255, 255, 255));
                        FillRect(hdcDst, &rect, whiteBrush);
                        DeleteObject(whiteBrush);
                        
                        // Copy the shared bitmap content
                        BitBlt(hdcDst, 0, 0, cx, cx, hdcSrc, 0, 0, SRCCOPY);
                        
                        SelectObject(hdcSrc, oldSrc);
                        SelectObject(hdcDst, oldDst);
                        
                        *phbmp = hRegularBmp;
                        hr = S_OK;
                    }
                    else
                    {
                        hr = E_FAIL;
                    }
                    
                    DeleteDC(hdcSrc);
                    DeleteDC(hdcDst);
                    ReleaseDC(nullptr, hdcScreen);
                }
                
                pSharedBitmap->Release();
            }

            pShellItem->Release();
        }

        pThumbCache->Release();
    }

    return hr;
}