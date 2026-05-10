[README.md](https://github.com/user-attachments/files/27565712/README.md)
# Always CD Ripper

**High Fidelity CD Ripper — YOUICHI SAIJO**

![version](https://img.shields.io/badge/version-1.0.0-4db8ff)
![platform](https://img.shields.io/badge/platform-Windows%2010%2F11-blue)
![license](https://img.shields.io/badge/license-GPL--3.0-green)

---

## 概要

Always CD Ripper は、CDを可能な限り正確に読み取り、高品質なFLACファイルとして保存するソフトウェアです。

**Always Player** とセットで使うことで「CDの完全な救済」を実現します。

```
Always CD Ripper  →  Always Player
正確に取り出す        忠実に鳴らす
```

---

## 主な機能

- **高精度読み取り** — セクタ単位で最大20回再読み取り・投票方式
- **傷の地図** — セクタマップでリアルタイム可視化
- **メタデータ自動取得** — CUEシートから自動取得
- **アルバムアート自動取得** — iTunes APIから自動ダウンロード
- **FLAC出力** — 日本語ファイル名・タグ完全対応
- **AccurateRipベリファイ** — リッピング品質を確認
- **5ステップウィザード** — Source → Output → Metadata → RIP → Verify

---

## 動作要件

- Windows 10 / 11（64bit）
- CDドライブ（外付けUSB対応）またはCUE/BINファイル
- インターネット接続（アルバムアート取得時）

---

## インストール

[Releases](https://github.com/YOUICHI-del/always-cd-ripper/releases) から
`AlwaysCDRipper_Setup.msi` をダウンロードしてダブルクリック。

---

## 使い方

### ① Source — ソース選択
CDドライブ または CUE/BINファイルを選択

### ② Output — 出力設定
出力先フォルダとフォーマット（FLAC推奨）を選択

### ③ Metadata — メタデータ確認
アルバム名・アーティスト・曲名・アルバムアートを確認・編集

### ④ RIP — リッピング実行
RIP STARTボタンで開始。セクタマップとログをリアルタイム表示

### ⑤ Verify — ベリファイ
AccurateRip DBと照合して品質を確認

---

## 出力フォルダ構成

```
出力先\
  └─ アーティスト名\
       └─ (年) アルバム名\
             ├─ 01. 曲名.flac
             ├─ 02. 曲名.flac
             └─ cover.jpg
```

---

## ビルド方法

### 必要環境
- Visual Studio 2022
- Qt 6.x (msvc2022_64)
- vcpkg

### 依存ライブラリ
```bat
C:\vcpkg\vcpkg install libflac taglib --triplet x64-windows
```

### ビルド
```bat
cmake -B build -S . -G "Visual Studio 17 2022" -A x64 ^
  -DCMAKE_PREFIX_PATH=C:/Qt/6.x.x/msvc2022_64 ^
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

---

## Always シリーズ

| ソフト | 説明 |
|---|---|
| [Always Player](https://github.com/YOUICHI-del/always-player) | 高音質オーディオプレーヤー |
| **Always CD Ripper** | 高精度CDリッパー |

---

## ライセンス

GPL-3.0 © 2026 YOUICHI SAIJO

---

## 作者

**YOUICHI SAIJO**
- GitHub: [@YOUICHI-del](https://github.com/YOUICHI-del)
- Always Player: [smartcube.sakuraweb.com](http://smartcube.sakuraweb.com/images/always-player/)
