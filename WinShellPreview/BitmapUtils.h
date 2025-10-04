#pragma once
#include "framework.h"

// Bitmap utility functions
HRESULT SaveHBITMAPAsPng(HBITMAP hbmp, LPCWSTR outPath);
HRESULT SaveBitmapAsBMP(HBITMAP hBitmap, LPCWSTR outputPath);
HRESULT SaveBitmapToFileImpl(HBITMAP hBitmap, LPCWSTR outputPath);
HBITMAP ConvertToCompatibleBitmap(HBITMAP hSourceBitmap, int width, int height);
HBITMAP CropFromTopLeft(HBITMAP hBitmap, int targetWidth, int targetHeight);

