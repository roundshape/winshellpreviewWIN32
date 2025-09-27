# WinShellPreview

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Windows](https://img.shields.io/badge/Platform-Windows-blue.svg)](https://microsoft.com/windows/)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)

Windows Shell APIを使用してファイルのプレビュー/サムネイルを生成するDLLライブラリです。Windows Explorerと同じプレビューハンドラーを使用して、様々なファイル形式のサムネイル画像を生成できます。

![Demo](https://via.placeholder.com/600x300/4CAF50/FFFFFF?text=WinShellPreview+Demo)

## 🚀 機能

- **Windows Shell API統合**: Windows Explorerと同じプレビューハンドラーを使用
- **多様なファイル形式対応**: PDF、Office文書（Word、Excel、PowerPoint）、画像、動画など
- **高品質サムネイル**: システム標準の高品質プレビューを生成
- **複数の取得方法**: フォールバック機能により確実な画像取得
- **画像形式対応**: PNG、JPG、BMP形式での保存
- **C++ API**: シンプルで使いやすいC++インターフェース
- **コマンドラインツール**: 単体テスト用のコマンドラインアプリケーション

## 📋 必要環境

- **OS**: Windows 10 (1903以降) または Windows 11
- **コンパイラ**: Visual Studio 2022 (v143) または Build Tools for Visual Studio 2022
- **CMake**: 3.20以上（CMakeビルドの場合）
- **ランタイム**: Visual C++ Redistributable for Visual Studio 2022

## 🛠️ 開発環境の構築

### 1. 必要なツールのインストール

#### Visual Studio 2022のインストール
```bash
# Chocolateyを使用する場合
choco install visualstudio2022community --params "--add Microsoft.VisualStudio.Workload.NativeDesktop"

# または、Visual Studio Installerから以下のワークロードをインストール：
# - C++によるデスクトップ開発
# - Windows 10/11 SDK (最新版)
```

#### CMakeのインストール
```bash
# Chocolateyを使用する場合
choco install cmake

# または、公式サイトからダウンロード
# https://cmake.org/download/
```

### 2. プロジェクトのクローンとビルド

```bash
# リポジトリをクローン
git clone https://github.com/roundshape/winshellpreviewWIN32.git
cd winshellpreviewWIN32

# ビルドディレクトリを作成
mkdir build
cd build

# CMakeでプロジェクトを生成（64bit Release）
cmake .. -A x64

# ビルド実行
cmake --build . --config Release

# または、MSBuildを直接使用
msbuild WinShellPreview.sln /p:Configuration=Release /p:Platform=x64
```

### 3. Visual Studioでの開発

```bash
# Visual StudioでCMakeプロジェクトを直接開く
# File > Open > CMake... から CMakeLists.txt を選択

# または、生成されたソリューションファイルを開く
start build/WinShellPreview.sln
```

## 📖 使用方法

### コマンドラインツール

ビルド後、`build/bin/Release/TestApp.exe` を使用してテストできます：

```bash
# 基本的な使用方法
TestApp.exe <input_file> <output_image> [size]

# 例：PDFファイルのサムネイル生成
TestApp.exe "C:\Documents\sample.pdf" "C:\Output\preview.png" 256

# 例：Office文書のサムネイル生成
TestApp.exe "C:\Documents\report.docx" "preview.jpg" 128

# 例：動画ファイルのサムネイル生成
TestApp.exe "C:\Videos\movie.mp4" "thumbnail.png" 512
```

### DLL APIの使用

#### 1. ヘッダーファイルのインクルード

```cpp
#include "WinShellPreview.h"

// 必要なライブラリをリンク
#pragma comment(lib, "WinShellPreview.lib")
#pragma comment(lib, "ole32.lib")
```

#### 2. 基本的な使用例

```cpp
#include <windows.h>
#include <iostream>
#include "WinShellPreview.h"

int main()
{
    // COMの初期化
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    
    HBITMAP hBitmap = nullptr;
    
    // サムネイルを取得（256x256ピクセル）
    HRESULT hr = GetFileThumbnail(L"C:\\test.pdf", 256, &hBitmap);
    
    if (SUCCEEDED(hr) && hBitmap) {
        // PNG形式で保存
        hr = SaveBitmapToFile(hBitmap, L"C:\\preview.png");
        
        if (SUCCEEDED(hr)) {
            std::cout << "サムネイルを保存しました！" << std::endl;
        }
        
        // リソースを解放
        ReleasePreviewBitmap(hBitmap);
    }
    
    // COMの終了処理
    CoUninitialize();
    return 0;
}
```

#### 3. カスタムサイズでのプレビュー生成

```cpp
// カスタムサイズでプレビューを生成
HBITMAP hBitmap = nullptr;
HRESULT hr = GetFilePreview(L"C:\\document.docx", 400, 300, &hBitmap);

if (SUCCEEDED(hr) && hBitmap) {
    // JPEG形式で保存
    SaveBitmapToFile(hBitmap, L"C:\\preview.jpg");
    ReleasePreviewBitmap(hBitmap);
}
```

## 📚 API リファレンス

### エクスポート関数

#### `GetFileThumbnail`
```cpp
HRESULT GetFileThumbnail(LPCWSTR filePath, UINT size, HBITMAP* phBitmap);
```
- **説明**: 指定されたファイルの正方形サムネイルを取得
- **パラメータ**:
  - `filePath`: ファイルパス（Unicode文字列）
  - `size`: サムネイルサイズ（ピクセル、正方形）
  - `phBitmap`: 出力されるHBITMAPハンドル
- **戻り値**: HRESULT（S_OKで成功）

#### `GetFilePreview`
```cpp
HRESULT GetFilePreview(LPCWSTR filePath, UINT width, UINT height, HBITMAP* phBitmap);
```
- **説明**: 指定されたサイズでファイルのプレビューを取得
- **パラメータ**:
  - `filePath`: ファイルパス（Unicode文字列）
  - `width`: 幅（ピクセル）
  - `height`: 高さ（ピクセル）
  - `phBitmap`: 出力されるHBITMAPハンドル
- **戻り値**: HRESULT（S_OKで成功）

#### `SaveBitmapToFile`
```cpp
HRESULT SaveBitmapToFile(HBITMAP hBitmap, LPCWSTR outputPath);
```
- **説明**: HBITMAPを画像ファイルとして保存
- **パラメータ**:
  - `hBitmap`: 保存するビットマップハンドル
  - `outputPath`: 出力ファイルパス（.png、.jpg、.bmp対応）
- **戻り値**: HRESULT（S_OKで成功）

#### `ReleasePreviewBitmap`
```cpp
void ReleasePreviewBitmap(HBITMAP hBitmap);
```
- **説明**: ビットマップリソースを解放
- **パラメータ**:
  - `hBitmap`: 解放するビットマップハンドル

## 🎯 対応ファイル形式

このライブラリはWindows Shell APIを使用するため、システムにインストールされているプレビューハンドラーに依存します。

### 標準対応形式
- **画像**: PNG, JPG, BMP, GIF, TIFF, WebP
- **文書**: PDF, DOC, DOCX, XLS, XLSX, PPT, PPTX, TXT, RTF
- **動画**: MP4, AVI, WMV, MOV, MKV
- **音声**: MP3, WAV, WMA（アルバムアート）
- **その他**: ZIP, RAR, EXE, DLL

### 追加対応形式
サードパーティ製プレビューハンドラーがインストールされている場合、追加の形式もサポートされます。

## 🔧 トラブルシューティング

### よくある問題と解決方法

#### 1. ビルドエラー
```bash
# CMakeキャッシュをクリア
rm -rf build/
mkdir build && cd build
cmake .. -A x64
```

#### 2. プレビュー生成に失敗する
- ファイルが存在することを確認
- ファイル形式に対応するプレビューハンドラーがインストールされているか確認
- ファイルが他のプロセスによってロックされていないか確認

#### 3. COMエラー
```cpp
// COMの初期化を忘れずに
CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

// 使用後は必ず終了処理を行う
CoUninitialize();
```

## 🏗️ プロジェクト構造

```
winshellpreviewWIN32/
├── CMakeLists.txt              # ルートCMakeファイル
├── README.md                   # このファイル
├── LICENSE                     # MITライセンス
├── .gitignore                  # Gitignore設定
├── WinShellPreview/            # メインDLLプロジェクト
│   ├── CMakeLists.txt
│   ├── WinShellPreview.h       # パブリックAPI
│   ├── WinShellPreview.cpp     # API実装
│   ├── PreviewHandler.h        # プレビューハンドラー
│   ├── PreviewHandler.cpp
│   ├── WinShellPreview.def     # DLLエクスポート定義
│   └── ...
├── TestApp/                    # テストアプリケーション
│   ├── CMakeLists.txt
│   └── main.cpp
└── build/                      # ビルド出力（git除外）
    ├── bin/Release/
    │   ├── WinShellPreview.dll
    │   └── TestApp.exe
    └── lib/Release/
        └── WinShellPreview.lib
```

## 🤝 コントリビューション

プルリクエストや課題の報告を歓迎します！

### 開発に参加する方法

1. このリポジトリをフォーク
2. フィーチャーブランチを作成 (`git checkout -b feature/AmazingFeature`)
3. 変更をコミット (`git commit -m 'Add some AmazingFeature'`)
4. ブランチにプッシュ (`git push origin feature/AmazingFeature`)
5. プルリクエストを作成

### コーディング規約

- C++17標準に準拠
- Windowsプラットフォーム固有のコード
- Unicode（UTF-16）文字列の使用
- COMオブジェクトの適切な管理

## 📄 ライセンス

このプロジェクトはMITライセンスの下で公開されています。詳細は[LICENSE](LICENSE)ファイルをご覧ください。

## 🙏 謝辞

- Microsoft Windows Shell API
- CMakeコミュニティ
- オープンソースコミュニティ

## 📞 サポート

- **Issues**: [GitHub Issues](https://github.com/roundshape/winshellpreviewWIN32/issues)
- **Discussions**: [GitHub Discussions](https://github.com/roundshape/winshellpreviewWIN32/discussions)

---

**注意**: このライブラリはWindows専用です。他のプラットフォームでは動作しません。