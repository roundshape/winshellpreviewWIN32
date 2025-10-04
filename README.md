# WinShellPreview

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Windows](https://img.shields.io/badge/Platform-Windows-blue.svg)](https://microsoft.com/windows/)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)

Windows Shell APIを使用してファイルのプレビュー/サムネイルを生成するDLLライブラリです。Windows Explorerと同じプレビューハンドラーを使用して、様々なファイル形式のサムネイル画像を生成できます。

![Demo](https://via.placeholder.com/600x300/4CAF50/FFFFFF?text=WinShellPreview+Demo)

## 🚀 機能

- **3つの明確なAPI**: サムネイル、プレビュー、アイコン取得を独立した関数で提供
- **Windows Shell API統合**: IThumbnailCache、IPreviewHandler、IShellItemImageFactoryを使用
- **スマートなトリミング**: 元画像のアスペクト比を自動取得して余白を削除
- **多様なファイル形式対応**: PDF、Office文書、画像、動画など
- **高速キャッシュ**: Windowsのサムネイルキャッシュシステムを活用
- **画像形式対応**: PNG、JPG、BMP形式での保存
- **C++ API**: シンプルで使いやすいC++インターフェース
- **コマンドラインツール**: モード切り替え可能なテストアプリケーション

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
TestApp.exe <モード> <input_file> <output_image> [size]

# モード:
#   -t, --thumbnail : サムネイル取得（キャッシュ使用、余白トリミング）
#   -p, --preview   : プレビュー取得（実際の内容表示）
#   -i, --icon      : アイコン取得（エクスプローラー風）

# 例：画像のサムネイル生成（余白自動削除）
TestApp.exe -t "C:\Pictures\photo.jpg" "thumbnail.png" 256

# 例：Office文書のプレビュー生成
TestApp.exe -p "C:\Documents\report.docx" "preview.png" 800

# 例：ファイルのアイコン取得
TestApp.exe -i "C:\Documents\sample.pdf" "icon.png" 128

# モード省略時はサムネイル取得
TestApp.exe "C:\test.jpg" "output.png" 256
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

#### 3. 3つの関数の使用例

```cpp
// サムネイル取得（高速、余白なし）
HBITMAP hThumb = nullptr;
GetFileThumbnail(L"C:\\photo.jpg", 256, &hThumb);
SaveBitmapToFile(hThumb, L"C:\\thumb.png");
ReleasePreviewBitmap(hThumb);

// プレビュー取得（実際の内容）
HBITMAP hPreview = nullptr;
GetFilePreview(L"C:\\document.docx", 800, 600, &hPreview);
SaveBitmapToFile(hPreview, L"C:\\preview.png");
ReleasePreviewBitmap(hPreview);

// アイコン取得（エクスプローラー風）
HBITMAP hIcon = nullptr;
GetFileIcon(L"C:\\file.txt", 128, &hIcon);
SaveBitmapToFile(hIcon, L"C:\\icon.png");
ReleasePreviewBitmap(hIcon);
```

#### 4. Electron + Koffi での使用例

```javascript
const koffi = require('koffi');
const path = require('path');

// DLLをロード
const dllPath = path.join(__dirname, 'WinShellPreview.dll');
const lib = koffi.load(dllPath);

// 関数を定義
const GetFileThumbnail = lib.func('int GetFileThumbnail(_Str16 filePath, uint size, _Out void** phBitmap)');
const GetFilePreview = lib.func('int GetFilePreview(_Str16 filePath, uint width, uint height, _Out void** phBitmap)');
const GetFileIcon = lib.func('int GetFileIcon(_Str16 filePath, uint size, _Out void** phBitmap)');
const SaveBitmapToFile = lib.func('int SaveBitmapToFile(void* hBitmap, _Str16 outputPath)');
const ReleasePreviewBitmap = lib.func('void ReleasePreviewBitmap(void* hBitmap)');

// ラッパー関数
async function getThumbnail(inputPath, outputPath, size = 256) {
    const hBitmap = [null];
    const hr = GetFileThumbnail(inputPath, size, hBitmap);
    
    if (hr !== 0) throw new Error(`Failed: 0x${hr.toString(16)}`);
    
    try {
        SaveBitmapToFile(hBitmap[0], outputPath);
    } finally {
        ReleasePreviewBitmap(hBitmap[0]);
    }
}

// 使用例
await getThumbnail('C:\\photo.jpg', 'C:\\thumb.png', 256);
```

## 📚 API リファレンス

### エクスポート関数

#### `GetFileThumbnail` - サムネイル取得

```cpp
HRESULT GetFileThumbnail(LPCWSTR filePath, UINT size, HBITMAP* phBitmap);
```

**説明**: ファイルのサムネイル画像を取得します。Windowsのサムネイルキャッシュを使用するため高速です。

**パラメータ**:
- `filePath`: ファイルパス（Unicode文字列）
- `size`: サムネイルの最大サイズ（ピクセル）
- `phBitmap`: 出力されるHBITMAPハンドル

**戻り値**: `S_OK (0)` で成功、その他はエラーコード

**動作**:
1. `IThumbnailCache`でキャッシュ確認（`WTS_INCACHEONLY`）
2. キャッシュになければ`WTS_EXTRACT`で生成
3. 失敗時は`IShellItemImageFactory`で取得
4. 元画像のアスペクト比を自動取得して余白をトリミング

**出力サイズ**: 元画像のアスペクト比を維持（例: 縦長画像 → 146x256）

**対応ファイル**: 画像、動画、Office文書、PDF等

---

#### `GetFilePreview` - プレビュー取得

```cpp
HRESULT GetFilePreview(LPCWSTR filePath, UINT width, UINT height, HBITMAP* phBitmap);
```

**説明**: ファイルの実際の内容をプレビュー画像として取得します。

**パラメータ**:
- `filePath`: ファイルパス（Unicode文字列）
- `width`: プレビューの幅（ピクセル）
- `height`: プレビューの高さ（ピクセル）
- `phBitmap`: 出力されるHBITMAPハンドル

**戻り値**: `S_OK (0)` で成功、その他はエラーコード

**動作**:
- `IPreviewHandler`を使用してファイルの実際の内容を描画
- フォールバックなし（プレビューハンドラーがない場合はエラー）

**対応ファイル**: Office文書、PDF、テキストファイル等（対応アプリのインストールが必要）

**注意**: サムネイルより時間がかかります（毎回レンダリングが必要）

---

#### `GetFileIcon` - アイコン取得（エクスプローラー風）

```cpp
HRESULT GetFileIcon(LPCWSTR filePath, UINT size, HBITMAP* phBitmap);
```

**説明**: Windowsエクスプローラーと同じロジックでアイコンまたはサムネイルを取得します。

**パラメータ**:
- `filePath`: ファイルパス（Unicode文字列）
- `size`: アイコンサイズ（ピクセル、正方形）
- `phBitmap`: 出力されるHBITMAPハンドル

**戻り値**: `S_OK (0)` で成功、その他はエラーコード

**動作**:
1. `IShellItemImageFactory::GetImage`で`SIIGBF_THUMBNAILONLY`を試行（画像ならサムネイル）
2. 失敗したら`SIIGBF_ICONONLY`でファイルタイプのアイコンを取得

**対応ファイル**: すべてのファイル

---

#### `SaveBitmapToFile` - ビットマップ保存

```cpp
HRESULT SaveBitmapToFile(HBITMAP hBitmap, LPCWSTR outputPath);
```

**説明**: HBITMAPを画像ファイルとして保存します。

**パラメータ**:
- `hBitmap`: 保存するビットマップハンドル
- `outputPath`: 出力ファイルパス（拡張子: `.png`, `.jpg`, `.bmp`）

**戻り値**: `S_OK (0)` で成功、その他はエラーコード

**形式**: PNG（`.png`）、BMP（その他）

---

#### `ReleasePreviewBitmap` - メモリ解放
```cpp
void ReleasePreviewBitmap(HBITMAP hBitmap);
```
- **説明**: ビットマップリソースを解放
- **パラメータ**:
  - `hBitmap`: 解放するビットマップハンドル

---

### 3つの関数の使い分け

| 関数 | 用途 | 速度 | 品質 | フォールバック |
|------|------|------|------|---------------|
| **GetFileThumbnail** | 画像・動画のサムネイル | ⚡ 高速（キャッシュ） | ⭐ 高品質、余白なし | あり |
| **GetFilePreview** | 文書の内容プレビュー | 🐢 遅い | ⭐⭐⭐ 実際の内容 | なし |
| **GetFileIcon** | エクスプローラー風表示 | ⚡ 高速 | ⭐⭐ 画像ならサムネイル | あり |

**推奨用途**:
- **画像ファイル**: `GetFileThumbnail` - 高速で余白なし
- **Office文書**: `GetFilePreview` - 実際の内容を表示
- **汎用的な表示**: `GetFileIcon` - 何でも対応

---

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