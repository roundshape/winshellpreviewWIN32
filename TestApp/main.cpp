#include <windows.h>
#include <iostream>
#include <string>
#include <locale>
#include <codecvt>
#include "WinShellPreview.h"

#pragma comment(lib, "ole32.lib")

void PrintUsage(const char* programName)
{
    std::cout << "Usage: " << programName << " <input_file> <output_image> [size]" << std::endl;
    std::cout << "  input_file   : Path to the file to generate preview" << std::endl;
    std::cout << "  output_image : Path to save the preview image (png/jpg/bmp)" << std::endl;
    std::cout << "  size        : Optional thumbnail size (default: 256)" << std::endl;
    std::cout << std::endl;
    std::cout << "Example:" << std::endl;
    std::cout << "  " << programName << " C:\\test.pdf C:\\preview.png 128" << std::endl;
}

int main(int argc, char* argv[])
{
    std::cout << "=== WinShellPreview Test Application ===" << std::endl;
    std::cout << "Build: " << __DATE__ << " " << __TIME__ << std::endl;
    std::cout << "Version: DEBUG_2024_0927" << std::endl;
    std::cout << std::endl;

    if (argc < 3)
    {
        PrintUsage(argv[0]);
        return 1;
    }

    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    std::string inputFile = argv[1];
    std::string outputFile = argv[2];
    UINT size = 256;

    if (argc >= 4)
    {
        size = std::stoul(argv[3]);
    }

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wInputFile = converter.from_bytes(inputFile);
    std::wstring wOutputFile = converter.from_bytes(outputFile);

    std::cout << "Generating preview for: " << inputFile << std::endl;
    std::cout << "Preview size: " << size << "x" << size << std::endl;

    HBITMAP hBitmap = nullptr;
    std::cout << "Trying GetFileThumbnail..." << std::endl;
    HRESULT hr = GetFileThumbnail(wInputFile.c_str(), size, &hBitmap);

    if (FAILED(hr))
    {
        std::cout << "GetFileThumbnail failed (0x" << std::hex << hr << std::dec << ")" << std::endl;
        
        // Try to provide more specific error information
        switch (hr)
        {
        case 0x8004b200:
            std::cout << "  -> WTS_E_EXTRACTNOTSUPPORTED: Preview handler doesn't support this operation" << std::endl;
            break;
        case 0x80070002:
            std::cout << "  -> ERROR_FILE_NOT_FOUND: File not found" << std::endl;
            break;
        case 0x80070005:
            std::cout << "  -> ERROR_ACCESS_DENIED: Access denied" << std::endl;
            break;
        case 0x80070057:
            std::cout << "  -> E_INVALIDARG: Invalid argument" << std::endl;
            break;
        case 0x80004005:
            std::cout << "  -> E_FAIL: Unspecified failure" << std::endl;
            break;
        case 0x80040154:
            std::cout << "  -> REGDB_E_CLASSNOTREG: Class not registered" << std::endl;
            break;
        case 0x800401F0:
            std::cout << "  -> CO_E_NOTINITIALIZED: COM not initialized" << std::endl;
            break;
        default:
            std::cout << "  -> Unknown error code" << std::endl;
            break;
        }
        
        std::cout << "Trying alternative preview method..." << std::endl;
        hr = GetFilePreview(wInputFile.c_str(), size, size, &hBitmap);
        
        if (FAILED(hr))
        {
            std::cout << "GetFilePreview also failed (0x" << std::hex << hr << std::dec << ")" << std::endl;
        }
    }
    else
    {
        std::cout << "GetFileThumbnail succeeded!" << std::endl;
    }

    std::cout << "Final result: hr=0x" << std::hex << hr << std::dec << ", hBitmap=" << (hBitmap ? "valid" : "null") << std::endl;

    if (SUCCEEDED(hr) && hBitmap)
    {
        std::cout << "Preview generated successfully!" << std::endl;

        // Verify bitmap is valid
        BITMAP bmp;
        if (GetObject(hBitmap, sizeof(BITMAP), &bmp))
        {
            std::cout << "Bitmap info: " << bmp.bmWidth << "x" << bmp.bmHeight << ", " << bmp.bmBitsPixel << " bits" << std::endl;
        }
        else
        {
            std::cout << "WARNING: GetObject failed, bitmap may be invalid" << std::endl;
        }

        std::cout << "About to save to: " << outputFile << std::endl;

        hr = SaveBitmapToFile(hBitmap, wOutputFile.c_str());
        if (SUCCEEDED(hr))
        {
            std::cout << "Preview saved to: " << outputFile << std::endl;
        }
        else
        {
            std::cerr << "Failed to save preview image. Error: 0x" << std::hex << hr << std::endl;
            std::cerr << "Output path: " << outputFile << std::endl;
        }

        ReleasePreviewBitmap(hBitmap);
    }
    else
    {
        std::cerr << "Failed to generate preview. Error: 0x" << std::hex << hr << std::endl;
        std::cerr << "Make sure the file exists and has a registered preview handler." << std::endl;
    }

    CoUninitialize();
    return SUCCEEDED(hr) ? 0 : 1;
}