#include <windows.h>
#include <iostream>
#include <string>
#include <locale>
#include <codecvt>
#include "WinShellPreview.h"

#pragma comment(lib, "ole32.lib")

void PrintUsage(const char* programName)
{
    std::cout << "Usage: " << programName << " <mode> <input_file> <output_image> [size]" << std::endl;
    std::cout << std::endl;
    std::cout << "Modes:" << std::endl;
    std::cout << "  -t, --thumbnail  : Get thumbnail (default, uses cache)" << std::endl;
    std::cout << "  -p, --preview    : Get preview (actual file content)" << std::endl;
    std::cout << "  -i, --icon       : Get file type icon" << std::endl;
    std::cout << std::endl;
    std::cout << "Arguments:" << std::endl;
    std::cout << "  input_file   : Path to the file" << std::endl;
    std::cout << "  output_image : Path to save the image (png/jpg/bmp)" << std::endl;
    std::cout << "  size         : Optional size (default: 256)" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << programName << " -t C:\\test.pdf C:\\thumb.png 256" << std::endl;
    std::cout << "  " << programName << " --preview C:\\doc.docx C:\\preview.png 800" << std::endl;
    std::cout << "  " << programName << " -i C:\\file.txt C:\\icon.png 128" << std::endl;
}

enum class Mode {
    Thumbnail,
    Preview,
    Icon
};

int main(int argc, char* argv[])
{
    std::cout << "=== WinShellPreview Test Application ===" << std::endl;
    std::cout << "Build: " << __DATE__ << " " << __TIME__ << std::endl;
    std::cout << "Version: 2.1 (with trimming)" << std::endl;
    std::cout << std::endl;

    if (argc < 3)
    {
        PrintUsage(argv[0]);
        return 1;
    }

    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    // Parse mode
    Mode mode = Mode::Thumbnail;  // Default
    int argOffset = 0;
    
    std::string firstArg = argv[1];
    if (firstArg == "-t" || firstArg == "--thumbnail")
    {
        mode = Mode::Thumbnail;
        argOffset = 1;
    }
    else if (firstArg == "-p" || firstArg == "--preview")
    {
        mode = Mode::Preview;
        argOffset = 1;
    }
    else if (firstArg == "-i" || firstArg == "--icon")
    {
        mode = Mode::Icon;
        argOffset = 1;
    }
    else if (firstArg[0] == '-')
    {
        std::cerr << "Unknown mode: " << firstArg << std::endl;
        PrintUsage(argv[0]);
        CoUninitialize();
        return 1;
    }

    if (argc < 3 + argOffset)
    {
        PrintUsage(argv[0]);
        CoUninitialize();
        return 1;
    }

    std::string inputFile = argv[1 + argOffset];
    std::string outputFile = argv[2 + argOffset];
    UINT size = 256;

    if (argc >= 4 + argOffset)
    {
        size = std::stoul(argv[3 + argOffset]);
    }

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wInputFile = converter.from_bytes(inputFile);
    std::wstring wOutputFile = converter.from_bytes(outputFile);

    std::cout << "Input file: " << inputFile << std::endl;
    std::cout << "Output file: " << outputFile << std::endl;
    std::cout << "Size: " << size << "x" << size << std::endl;

    HBITMAP hBitmap = nullptr;
    HRESULT hr = E_FAIL;

    // Execute based on mode
    switch (mode)
    {
    case Mode::Thumbnail:
        std::cout << "Mode: Thumbnail (using IThumbnailCache)" << std::endl;
        hr = GetFileThumbnail(wInputFile.c_str(), size, &hBitmap);
        break;

    case Mode::Preview:
        std::cout << "Mode: Preview (using IPreviewHandler)" << std::endl;
        hr = GetFilePreview(wInputFile.c_str(), size, size, &hBitmap);
        break;

    case Mode::Icon:
        std::cout << "Mode: Icon (using SHGetFileInfo)" << std::endl;
        hr = GetFileIcon(wInputFile.c_str(), size, &hBitmap);
        break;
    }

    if (FAILED(hr))
    {
        std::cout << "Failed with error code: 0x" << std::hex << hr << std::dec << std::endl;
        
        // Provide more specific error information
        switch (hr)
        {
        case 0x8004b200:
            std::cout << "  -> WTS_E_EXTRACTNOTSUPPORTED: Handler doesn't support this operation" << std::endl;
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
    }
    else
    {
        std::cout << "Success!" << std::endl;
    }

    std::cout << "Final result: hr=0x" << std::hex << hr << std::dec << ", hBitmap=" << (hBitmap ? "valid" : "null") << std::endl;

    if (SUCCEEDED(hr) && hBitmap)
    {
        std::cout << "Image generated successfully!" << std::endl;

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
            std::cout << "Image saved to: " << outputFile << std::endl;
        }
        else
        {
            std::cerr << "Failed to save image. Error: 0x" << std::hex << hr << std::endl;
            std::cerr << "Output path: " << outputFile << std::endl;
        }

        ReleasePreviewBitmap(hBitmap);
    }
    else
    {
        std::cerr << "Failed to generate image. Error: 0x" << std::hex << hr << std::endl;
        std::cerr << "Make sure the file exists and the appropriate handler is registered." << std::endl;
    }

    CoUninitialize();
    return SUCCEEDED(hr) ? 0 : 1;
}