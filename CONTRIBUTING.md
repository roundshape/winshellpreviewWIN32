# コントリビューションガイド

WinShellPreviewプロジェクトへのコントリビューションを歓迎します！

## 開発環境のセットアップ

1. **必要なツール**
   - Visual Studio 2022 (Community版以上)
   - CMake 3.20以上
   - Git

2. **リポジトリのフォーク**
   ```bash
   # GitHubでリポジトリをフォーク後
   git clone https://github.com/YOUR_USERNAME/winshellpreviewWIN32.git
   cd winshellpreviewWIN32
   ```

3. **ビルド**
   ```bash
   mkdir build
   cd build
   cmake .. -A x64
   cmake --build . --config Release
   ```

## コントリビューションの流れ

1. **Issue の確認**
   - 既存のIssueを確認し、重複を避ける
   - 新機能や大きな変更の場合は事前にIssueで議論

2. **ブランチの作成**
   ```bash
   git checkout -b feature/your-feature-name
   ```

3. **コーディング**
   - コーディング規約に従う（下記参照）
   - 適切なコメントを追加
   - テストを実行

4. **コミット**
   ```bash
   git add .
   git commit -m "feat: 新機能の説明"
   ```

5. **プルリクエスト**
   - 明確なタイトルと説明を記載
   - 変更内容の詳細を説明
   - 関連するIssue番号を記載

## コーディング規約

### C++コード
- **スタイル**: Microsoft C++ スタイルガイドに準拠
- **インデント**: 4スペース（タブ禁止）
- **命名規則**:
  - クラス名: PascalCase (`PreviewHandler`)
  - 関数名: PascalCase (`GetFileThumbnail`)
  - 変数名: camelCase (`hBitmap`)
  - 定数: UPPER_CASE (`MAX_SIZE`)

### ファイル構造
- **ヘッダー**: `.h` 拡張子
- **実装**: `.cpp` 拡張子
- **プリコンパイルヘッダー**: `pch.h` / `pch.cpp`

### コメント
```cpp
/**
 * @brief ファイルのサムネイルを取得する
 * @param filePath ファイルパス
 * @param size サムネイルサイズ（ピクセル）
 * @param phBitmap 出力ビットマップハンドル
 * @return HRESULT 成功時はS_OK
 */
HRESULT GetFileThumbnail(LPCWSTR filePath, UINT size, HBITMAP* phBitmap);
```

## テスト

### 単体テスト
```bash
# TestAppでの基本テスト
cd build/bin/Release
TestApp.exe "C:\test.pdf" "output.png" 256
```

### 対応形式テスト
以下の形式で動作確認を行ってください：
- PDF (.pdf)
- Office文書 (.docx, .xlsx, .pptx)
- 画像 (.png, .jpg, .bmp)
- 動画 (.mp4, .avi)

## プルリクエストのガイドライン

### タイトル
- `feat:` 新機能追加
- `fix:` バグ修正
- `docs:` ドキュメント更新
- `refactor:` リファクタリング
- `test:` テスト追加・修正

### 説明
```markdown
## 概要
変更内容の簡潔な説明

## 変更内容
- 具体的な変更点1
- 具体的な変更点2

## テスト
- 実施したテスト内容
- 確認した環境

## 関連Issue
Fixes #123
```

## バグレポート

### Issue テンプレート
```markdown
## バグの概要
簡潔な説明

## 再現手順
1. 
2. 
3. 

## 期待する動作
期待していた結果

## 実際の動作
実際に発生した結果

## 環境
- OS: Windows 11 22H2
- Visual Studio: 2022 17.8.0
- CMake: 3.28.0

## 追加情報
スクリーンショットやログなど
```

## 質問・提案

- **GitHub Discussions** を使用
- **Issue** で新機能提案
- **Discord** での議論（予定）

## ライセンス

コントリビューションは MIT ライセンスの下で提供されます。

## 謝辞

すべてのコントリビューターに感謝します！
