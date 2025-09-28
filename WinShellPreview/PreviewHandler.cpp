#include "pch.h"
#include "PreviewHandler.h"
#include <commoncontrols.h>
#include <shellapi.h>
#include <shobjidl.h>
#include <shlguid.h>
#include <propsys.h>
#include <shlwapi.h>

#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "shell32.lib")

// Define GUIDs for preview handler
static const GUID GUID_BHID_PreviewHandler = {0x7f73be3f, 0xfb79, 0x493c, {0xa6, 0xc7, 0x7e, 0xe1, 0x4e, 0x24, 0x5a, 0x19}};
static const GUID GUID_IInitializeWithFile = {0xb7d14566, 0x0509, 0x4cce, {0xa7, 0x1f, 0x0a, 0x55, 0x42, 0x33, 0xbd, 0x9b}};

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

    // Try IPreviewHandler first (for actual file content preview)
    HRESULT hr = GetPreviewUsingIPreviewHandler(pszFilePath, cx, cx, phbmp);
    if (SUCCEEDED(hr))
    {
        if (pdwAlpha) *pdwAlpha = WTSAT_UNKNOWN;
        return hr;
    }
    
    printf("IPreviewHandler failed, trying IShellItemImageFactory...\n");

    // Try IShellItemImageFactory second (recommended modern approach for thumbnails)
    hr = GetImageUsingIShellItemImageFactory(pszFilePath, cx, phbmp);
    if (SUCCEEDED(hr))
    {
        if (pdwAlpha) *pdwAlpha = WTSAT_UNKNOWN;
        return hr;
    }
    
    printf("IShellItemImageFactory failed, trying fallback methods...\n");

    // Try thumbnail cache second (often works when Explorer preview works)
    hr = GetThumbnailUsingIThumbnailCache(pszFilePath, cx, phbmp);
    if (SUCCEEDED(hr))
    {
        if (pdwAlpha) *pdwAlpha = WTSAT_UNKNOWN;
        return hr;
    }
    
    // Debug: IThumbnailCache failed
    OutputDebugStringA("IThumbnailCache failed\n");

    // Try IThumbnailProvider third (most reliable for modern file types)
    hr = GetThumbnailUsingIThumbnailProvider(pszFilePath, cx, phbmp, pdwAlpha);
    if (SUCCEEDED(hr))
    {
        return hr;
    }
    
    // Debug: IThumbnailProvider failed
    OutputDebugStringA("IThumbnailProvider failed\n");

    // Try IExtractImage last (classic method, works well with shell extensions)
    hr = GetThumbnailUsingIExtractImage(pszFilePath, cx, cx, phbmp);
    if (SUCCEEDED(hr) && pdwAlpha)
    {
        *pdwAlpha = WTSAT_UNKNOWN;
    }
    
    // Debug: IExtractImage result
    if (FAILED(hr))
    {
        OutputDebugStringA("IExtractImage also failed\n");
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
    char debugMsg[256];
    
    IShellItem* pShellItem = nullptr;
    HRESULT hr = SHCreateItemFromParsingName(pszFilePath, nullptr, IID_PPV_ARGS(&pShellItem));
    
    sprintf_s(debugMsg, "IThumbnailProvider: SHCreateItemFromParsingName returned 0x%08x\n", hr);
    OutputDebugStringA(debugMsg);
    printf("%s", debugMsg);

    if (SUCCEEDED(hr))
    {
        IThumbnailProvider* pThumbProvider = nullptr;
        hr = pShellItem->BindToHandler(nullptr, BHID_ThumbnailHandler, IID_PPV_ARGS(&pThumbProvider));
        
        sprintf_s(debugMsg, "IThumbnailProvider: BindToHandler returned 0x%08x\n", hr);
        OutputDebugStringA(debugMsg);
        printf("%s", debugMsg);

        if (SUCCEEDED(hr))
        {
            hr = pThumbProvider->GetThumbnail(cx, phbmp, pdwAlpha);
            
            sprintf_s(debugMsg, "IThumbnailProvider: GetThumbnail returned 0x%08x\n", hr);
            OutputDebugStringA(debugMsg);
            printf("%s", debugMsg);
            
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

    char debugMsg[256];
    
    IShellFolder* pDesktop = nullptr;
    HRESULT hr = SHGetDesktopFolder(&pDesktop);
    
    sprintf_s(debugMsg, "IExtractImage: SHGetDesktopFolder returned 0x%08x\n", hr);
    OutputDebugStringA(debugMsg);

    if (SUCCEEDED(hr))
    {
        PIDLIST_ABSOLUTE pidl = nullptr;
        hr = pDesktop->ParseDisplayName(nullptr, nullptr, const_cast<LPWSTR>(pszFilePath), nullptr, &pidl, nullptr);
        
        sprintf_s(debugMsg, "IExtractImage: ParseDisplayName returned 0x%08x\n", hr);
        OutputDebugStringA(debugMsg);

        if (SUCCEEDED(hr))
        {
            IShellFolder* pFolder = nullptr;
            PCUITEMID_CHILD pidlChild = nullptr;
            hr = SHBindToParent(pidl, IID_IShellFolder, (void**)&pFolder, &pidlChild);
            
            sprintf_s(debugMsg, "IExtractImage: SHBindToParent returned 0x%08x\n", hr);
            OutputDebugStringA(debugMsg);

            if (SUCCEEDED(hr))
            {
                IExtractImage* pExtract = nullptr;
                hr = pFolder->GetUIObjectOf(nullptr, 1, &pidlChild, IID_IExtractImage, nullptr, (void**)&pExtract);
                
                sprintf_s(debugMsg, "IExtractImage: GetUIObjectOf returned 0x%08x\n", hr);
                OutputDebugStringA(debugMsg);

                if (SUCCEEDED(hr))
                {
                    SIZE size = { (LONG)cx, (LONG)cy };
                    DWORD dwPriority = 0;
                    DWORD dwFlags = IEIFLAG_SCREEN | IEIFLAG_OFFLINE;
                    WCHAR szPath[MAX_PATH] = { 0 };

                    hr = pExtract->GetLocation(szPath, MAX_PATH, &dwPriority, &size, 32, &dwFlags);
                    
                    sprintf_s(debugMsg, "IExtractImage: GetLocation returned 0x%08x\n", hr);
                    OutputDebugStringA(debugMsg);

                    if (SUCCEEDED(hr))
                    {
                        hr = pExtract->Extract(phbmp);
                        
                        sprintf_s(debugMsg, "IExtractImage: Extract returned 0x%08x\n", hr);
                        OutputDebugStringA(debugMsg);
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
    
    char debugMsg[256];
    sprintf_s(debugMsg, "IThumbnailCache: CoCreateInstance returned 0x%08x\n", hr);
    OutputDebugStringA(debugMsg);
    printf("%s", debugMsg);

    if (SUCCEEDED(hr))
    {
        IShellItem* pShellItem = nullptr;
        hr = SHCreateItemFromParsingName(pszFilePath, nullptr, IID_PPV_ARGS(&pShellItem));
        
        sprintf_s(debugMsg, "IThumbnailCache: SHCreateItemFromParsingName returned 0x%08x\n", hr);
        OutputDebugStringA(debugMsg);
        printf("%s", debugMsg);

        if (SUCCEEDED(hr))
        {
            ISharedBitmap* pSharedBitmap = nullptr;
            WTS_CACHEFLAGS cacheFlags;
            WTS_THUMBNAILID thumbId;

            hr = pThumbCache->GetThumbnail(pShellItem, cx, WTS_EXTRACT, &pSharedBitmap, &cacheFlags, &thumbId);
            
            sprintf_s(debugMsg, "IThumbnailCache: GetThumbnail returned 0x%08x\n", hr);
            OutputDebugStringA(debugMsg);
            printf("%s", debugMsg);

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

HRESULT PreviewHandler::GetImageUsingIShellItemImageFactory(LPCWSTR pszFilePath, UINT cx, HBITMAP* phbmp)
{
    if (!pszFilePath || !phbmp)
        return E_INVALIDARG;
    
    *phbmp = nullptr;
    
    char debugMsg[256];
    
    // Create IShellItem from file path
    IShellItem* pShellItem = nullptr;
    HRESULT hr = SHCreateItemFromParsingName(pszFilePath, nullptr, IID_PPV_ARGS(&pShellItem));
    
    sprintf_s(debugMsg, "IShellItemImageFactory: SHCreateItemFromParsingName returned 0x%08x\n", hr);
    OutputDebugStringA(debugMsg);
    printf("%s", debugMsg);
    
    if (SUCCEEDED(hr))
    {
        // Query for IShellItemImageFactory interface
        IShellItemImageFactory* pImageFactory = nullptr;
        hr = pShellItem->QueryInterface(IID_PPV_ARGS(&pImageFactory));
        
        sprintf_s(debugMsg, "IShellItemImageFactory: QueryInterface returned 0x%08x\n", hr);
        OutputDebugStringA(debugMsg);
        printf("%s", debugMsg);
        
        if (SUCCEEDED(hr))
        {
            // Set up parameters for image generation
            SIZE size = {(LONG)cx, (LONG)cx};
            DWORD flags = SIIGBF_THUMBNAILONLY | SIIGBF_BIGGERSIZEOK | SIIGBF_RESIZETOFIT;
            
            // Try with thumbnail only first
            hr = pImageFactory->GetImage(size, flags, phbmp);
            
            sprintf_s(debugMsg, "IShellItemImageFactory: GetImage (THUMBNAILONLY) returned 0x%08x\n", hr);
            OutputDebugStringA(debugMsg);
            printf("%s", debugMsg);
            
            // If thumbnail-only failed, try without the THUMBNAILONLY flag (allows icons)
            if (FAILED(hr))
            {
                flags = SIIGBF_BIGGERSIZEOK | SIIGBF_RESIZETOFIT;
                hr = pImageFactory->GetImage(size, flags, phbmp);
                
                sprintf_s(debugMsg, "IShellItemImageFactory: GetImage (fallback) returned 0x%08x\n", hr);
                OutputDebugStringA(debugMsg);
                printf("%s", debugMsg);
            }
            
            pImageFactory->Release();
        }
        
        pShellItem->Release();
    }
    
    return hr;
}

HRESULT PreviewHandler::GetPreviewUsingIPreviewHandler(LPCWSTR pszFilePath, UINT cx, UINT cy, HBITMAP* phbmp)
{
    if (!pszFilePath || !phbmp)
        return E_INVALIDARG;
    
    *phbmp = nullptr;
    
    char debugMsg[256];
    
    // Check if current thread is already COM initialized
    HRESULT hrCom = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    
    if (hrCom == RPC_E_CHANGED_MODE)
    {
        sprintf_s(debugMsg, "IPreviewHandler: Thread is MTA - creating dedicated STA thread\n");
        OutputDebugStringA(debugMsg);
        printf("%s", debugMsg);
        
        // Create dedicated STA thread for preview
        PreviewThreadData threadData = {};
        threadData.filePath = pszFilePath;
        threadData.width = cx;
        threadData.height = cy;
        threadData.pBitmap = phbmp;
        threadData.result = E_FAIL;
        threadData.completionEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
        
        if (!threadData.completionEvent)
            return E_FAIL;
        
        HANDLE hThread = CreateThread(nullptr, 0, PreviewSTAThread, &threadData, 0, nullptr);
        
        if (hThread)
        {
            // Wait for completion (30 second timeout)
            DWORD waitResult = WaitForSingleObject(threadData.completionEvent, 30000);
            
            if (waitResult == WAIT_OBJECT_0)
            {
                sprintf_s(debugMsg, "IPreviewHandler: STA thread completed with result 0x%08x\n", threadData.result);
                OutputDebugStringA(debugMsg);
                printf("%s", debugMsg);
                
                CloseHandle(hThread);
                CloseHandle(threadData.completionEvent);
                return threadData.result;
            }
            else
            {
                sprintf_s(debugMsg, "IPreviewHandler: STA thread timeout or error\n");
                OutputDebugStringA(debugMsg);
                printf("%s", debugMsg);
                
                TerminateThread(hThread, 0);
                CloseHandle(hThread);
                CloseHandle(threadData.completionEvent);
                return E_FAIL;
            }
        }
        
        CloseHandle(threadData.completionEvent);
        return E_FAIL;
    }
    else if (FAILED(hrCom))
    {
        sprintf_s(debugMsg, "IPreviewHandler: CoInitialize failed 0x%08x\n", hrCom);
        OutputDebugStringA(debugMsg);
        printf("%s", debugMsg);
        return hrCom;
    }
    
    sprintf_s(debugMsg, "IPreviewHandler: Using current STA thread\n");
    OutputDebugStringA(debugMsg);
    printf("%s", debugMsg);
    
    // Continue with current thread - call the STA implementation directly
    HRESULT hr = GetPreviewInSTAThread(pszFilePath, cx, cy, phbmp);
    
    if (SUCCEEDED(hrCom))
    {
        CoUninitialize();
    }
    
    return hr;
}

HRESULT PreviewHandler::GetPreviewInSTAThread(LPCWSTR pszFilePath, UINT cx, UINT cy, HBITMAP* phbmp)
{
    if (!pszFilePath || !phbmp)
        return E_INVALIDARG;
    
    *phbmp = nullptr;
    
    char debugMsg[256];
    
    // Get preview handler using CLSID + LOCAL_SERVER approach
    IPreviewHandler* pPreviewHandler = nullptr;
    HRESULT hr = GetPreviewHandlerFromExtension(pszFilePath, &pPreviewHandler);
    
    if (FAILED(hr))
    {
        sprintf_s(debugMsg, "IPreviewHandler: GetPreviewHandlerFromExtension failed 0x%08x\n", hr);
        OutputDebugStringA(debugMsg);
        printf("%s", debugMsg);
        return hr;
    }
    
    sprintf_s(debugMsg, "IPreviewHandler: Preview handler created successfully\n");
    OutputDebugStringA(debugMsg);
    printf("%s", debugMsg);
    
    // Create a visible window for hosting the preview (Excel requires visible window)
    // Place it off-screen to avoid user distraction
    HWND hwndParent = CreateWindowExW(
        0,
        L"STATIC",
        L"PreviewHost",
        WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        -10000, -10000, cx, cy,  // Off-screen coordinates
        nullptr, nullptr, GetModuleHandle(nullptr), nullptr
    );
    
    if (!hwndParent)
    {
        pPreviewHandler->Release();
        return E_FAIL;
    }
    
    sprintf_s(debugMsg, "IPreviewHandler: CreateWindow hwnd=0x%p\n", hwndParent);
    OutputDebugStringA(debugMsg);
    printf("%s", debugMsg);
    
    // Show window as visible but not activated (Excel requires visible window)
    ShowWindow(hwndParent, SW_SHOWNOACTIVATE);
    UpdateWindow(hwndParent);
    
    // Initialize the preview handler with file
    IInitializeWithFile* pInitFile = nullptr;
    hr = pPreviewHandler->QueryInterface(IID_IInitializeWithFile, (void**)&pInitFile);
    
    if (SUCCEEDED(hr))
    {
        hr = pInitFile->Initialize(pszFilePath, STGM_READ | STGM_SHARE_DENY_NONE);
        pInitFile->Release();
        
        sprintf_s(debugMsg, "IPreviewHandler: Initialize returned 0x%08x\n", hr);
        OutputDebugStringA(debugMsg);
        printf("%s", debugMsg);
    }
    
    if (SUCCEEDED(hr))
    {
        // Set the window and rectangle
        RECT rc = {0, 0, (LONG)cx, (LONG)cy};
        hr = pPreviewHandler->SetWindow(hwndParent, &rc);
        
        if (SUCCEEDED(hr))
        {
            hr = pPreviewHandler->SetRect(&rc);
            
            if (SUCCEEDED(hr))
            {
                hr = pPreviewHandler->DoPreview();
                
                sprintf_s(debugMsg, "IPreviewHandler: DoPreview returned 0x%08x\n", hr);
                OutputDebugStringA(debugMsg);
                printf("%s", debugMsg);
                
                if (SUCCEEDED(hr))
                {
                    // Message pump and capture (simplified for now)
                    Sleep(2000); // Simple wait for now
                    
                    // Create bitmap for capture
                    HDC hdcScreen = GetDC(nullptr);
                    HDC hdcMem = CreateCompatibleDC(hdcScreen);
                    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, cx, cy);
                    
                    if (hBitmap)
                    {
                        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);
                        PrintWindow(hwndParent, hdcMem, PW_RENDERFULLCONTENT);
                        SelectObject(hdcMem, hOldBitmap);
                        *phbmp = hBitmap;
                        hr = S_OK;
                    }
                    
                    DeleteDC(hdcMem);
                    ReleaseDC(nullptr, hdcScreen);
                }
            }
        }
    }
    
    DestroyWindow(hwndParent);
    pPreviewHandler->Release();
    
    return hr;
}

// STA thread function for preview handling
DWORD WINAPI PreviewHandler::PreviewSTAThread(LPVOID lpParam)
{
    PreviewThreadData* pData = static_cast<PreviewThreadData*>(lpParam);
    
    char debugMsg[256];
    sprintf_s(debugMsg, "IPreviewHandler: STA thread started\n");
    OutputDebugStringA(debugMsg);
    printf("%s", debugMsg);
    
    // Initialize COM in STA mode
    HRESULT hrCom = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hrCom))
    {
        sprintf_s(debugMsg, "IPreviewHandler: STA thread CoInitialize failed 0x%08x\n", hrCom);
        OutputDebugStringA(debugMsg);
        printf("%s", debugMsg);
        
        pData->result = hrCom;
        SetEvent(pData->completionEvent);
        return 1;
    }
    
    // Create temporary PreviewHandler instance for the actual work
    PreviewHandler handler;
    
    // Call the actual preview implementation (we'll implement this next)
    pData->result = handler.GetPreviewInSTAThread(pData->filePath, pData->width, pData->height, pData->pBitmap);
    
    CoUninitialize();
    SetEvent(pData->completionEvent);
    
    sprintf_s(debugMsg, "IPreviewHandler: STA thread completed with result 0x%08x\n", pData->result);
    OutputDebugStringA(debugMsg);
    printf("%s", debugMsg);
    
    return 0;
}

// Get preview handler from file extension using CLSID + CLSCTX_LOCAL_SERVER
HRESULT PreviewHandler::GetPreviewHandlerFromExtension(LPCWSTR pszFilePath, IPreviewHandler** ppPreviewHandler)
{
    if (!pszFilePath || !ppPreviewHandler)
        return E_INVALIDARG;
    
    *ppPreviewHandler = nullptr;
    
    char debugMsg[256];
    
    // Get file extension
    LPCWSTR pszExt = wcsrchr(pszFilePath, L'.');
    if (!pszExt)
        return E_FAIL;
    
    sprintf_s(debugMsg, "IPreviewHandler: Getting CLSID for extension %S\n", pszExt);
    OutputDebugStringA(debugMsg);
    printf("%s", debugMsg);
    
    // Get preview handler CLSID from extension
    WCHAR szCLSID[MAX_PATH] = {};
    DWORD dwSize = MAX_PATH;
    
    HRESULT hr = AssocQueryStringW(
        ASSOCF_NONE,
        ASSOCSTR_SHELLEXTENSION,
        pszExt,
        L"{8895b1c6-b41f-4c1c-a562-0d564250836f}", // BHID_PreviewHandler
        szCLSID,
        &dwSize
    );
    
    if (FAILED(hr))
    {
        sprintf_s(debugMsg, "IPreviewHandler: AssocQueryString failed 0x%08x\n", hr);
        OutputDebugStringA(debugMsg);
        printf("%s", debugMsg);
        return hr;
    }
    
    sprintf_s(debugMsg, "IPreviewHandler: Found CLSID %S\n", szCLSID);
    OutputDebugStringA(debugMsg);
    printf("%s", debugMsg);
    
    // Convert string to CLSID
    CLSID clsid;
    hr = CLSIDFromString(szCLSID, &clsid);
    if (FAILED(hr))
    {
        sprintf_s(debugMsg, "IPreviewHandler: CLSIDFromString failed 0x%08x\n", hr);
        OutputDebugStringA(debugMsg);
        printf("%s", debugMsg);
        return hr;
    }
    
    // Create preview handler using local server
    hr = CoCreateInstance(clsid, nullptr, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(ppPreviewHandler));
    
    sprintf_s(debugMsg, "IPreviewHandler: CoCreateInstance (LOCAL_SERVER) returned 0x%08x\n", hr);
    OutputDebugStringA(debugMsg);
    printf("%s", debugMsg);
    
    return hr;
}