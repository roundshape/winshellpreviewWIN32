# テストアプリケーションの設定
add_executable(TestApp main.cpp)

# コンパイル定義
target_compile_definitions(TestApp PRIVATE
    _CONSOLE
)

# WinShellPreviewライブラリをリンク
target_link_libraries(TestApp PRIVATE
    WinShellPreview
    ole32
)

# インストール設定
install(TARGETS TestApp
    RUNTIME DESTINATION bin
)