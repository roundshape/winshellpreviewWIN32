# DLLライブラリの設定
set(SOURCES
    dllmain.cpp
    pch.cpp
    PreviewHandler.cpp
    WinShellPreview.cpp
    Thumbnail.cpp
    Preview.cpp
    Icon.cpp
    BitmapUtils.cpp
)

set(HEADERS
    framework.h
    pch.h
    PreviewHandler.h
    WinShellPreview.h
    ThumbnailImpl.h
    PreviewImpl.h
    IconImpl.h
    BitmapUtils.h
)

# DLLを作成
add_library(WinShellPreview SHARED ${SOURCES} ${HEADERS} WinShellPreview.def)

# プリコンパイルドヘッダーの設定
target_precompile_headers(WinShellPreview PRIVATE pch.h)

# コンパイル定義
target_compile_definitions(WinShellPreview PRIVATE
    WINSHELLPREVIEW_EXPORTS
    _WINDOWS
    _USRDLL
)

# UTF-8サポートを追加して警告を抑制
target_compile_options(WinShellPreview PRIVATE
    /utf-8
)

# 必要なライブラリをリンク
target_link_libraries(WinShellPreview PRIVATE
    shlwapi
    ole32
    oleaut32
    uuid
    shell32
    gdiplus
    msimg32
)

# インクルードディレクトリを設定（他のプロジェクトが使用するため）
target_include_directories(WinShellPreview PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# インストール設定
install(TARGETS WinShellPreview
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES WinShellPreview.h
    DESTINATION include
)