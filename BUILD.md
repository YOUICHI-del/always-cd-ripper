# Always CD Ripper — ビルド手順（Win32版）

## 必要環境
- Windows 11 / 10
- Visual Studio 2022（C++デスクトップ開発ワークロード）
- CMake 3.20以上
- Qt 6.5以上
- vcpkg（C:\vcpkg にインストール済み）

---

## 1. vcpkgで依存ライブラリをインストール

```bat
C:\vcpkg\vcpkg install libflac taglib --triplet x64-windows
```

※ libcdio は不要。CD読み取りはWindows標準API（Win32 IOCTL）で実装済み。

---

## 2. ZIPを展開

例: `C:\Projects\AlwaysCDRipper`

---

## 3. Visual Studio 2022 で開く

1. VS2022 起動
2. 「ファイル」→「CMakeプロジェクトを開く」
3. `CMakeLists.txt` を選択

---

## 4. CMake設定（CMakeSettings.json）

VS2022 上部の歯車アイコン → 以下を設定：

```json
{
  "configurations": [
    {
      "name": "x64-Release",
      "generator": "Visual Studio 17 2022",
      "configurationType": "Release",
      "buildRoot": "${projectDir}\\build",
      "variables": [
        {
          "name": "CMAKE_TOOLCHAIN_FILE",
          "value": "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
        },
        {
          "name": "CMAKE_PREFIX_PATH",
          "value": "C:/Qt/6.x.x/msvc2022_64"
        }
      ]
    }
  ]
}
```

`C:/Qt/6.x.x` はインストールしたQtバージョンに合わせて変更。

---

## 5. ビルド

メニュー「ビルド」→「すべてビルド」または Ctrl+Shift+B

---

## アーキテクチャ

```
AlwaysCDRipper/
  src/
    ripper/
      CdDrive.cpp/h          ← Win32 IOCTL でTOC・RAW読み取り
      ParanoiaReader.cpp/h   ← 多重読み取り・投票方式で信頼性確保
      FlacEncoder.cpp/h      ← libFLAC でエンコード
    metadata/
      MusicBrainz.cpp/h      ← MusicBrainz API
      CoverArt.cpp/h         ← Cover Art Archive
      TagWriter.cpp/h        ← TagLib でFLACタグ書き込み
    ui/
      MainWindow.cpp/h       ← メインウィンドウ
      SectorMap.cpp/h        ← 傷の地図ウィジェット
      MetadataEditor.cpp/h   ← メタデータ編集
      LogViewer.cpp/h        ← 儀式的ログ
```
