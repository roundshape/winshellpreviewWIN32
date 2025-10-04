#pragma once
#include "framework.h"

// Thumbnail implementation
HRESULT GetFileThumbnailImpl(LPCWSTR filePath, UINT size, HBITMAP* phBitmap);

// Helper function (thumbnail-specific)
HRESULT GetMediaDimensions(LPCWSTR filePath, UINT* width, UINT* height);

