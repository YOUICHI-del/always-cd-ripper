Always CD Ripper
High Fidelity CD Ripper — YOUICHI SAIJO
![version](https://img.shields.io/badge/version-1.0.0-blue)
![platform](https://img.shields.io/badge/platform-Windows%2010%2F11-blue)
![license](https://img.shields.io/badge/license-GPL--3.0-green)
---
概要
Always CD Ripper は、CDを可能な限り正確に読み取り、高品質な FLAC / WAV ファイルとして保存するソフトウェアです。
Always Player とセットで使うことで「CDの完全な救済」を実現します。
```
Always CD Ripper  →  Always Player
正確に取り出す        忠実に鳴らす
```
---
主な機能
高精度読み取り — セクタ単位で最大20回再読み取り・投票方式
ドライブキャッシュ無効化 — 隣接セクタ読み込みによるキャッシュ掃き出し処理で投票方式を確実に機能させる
傷の地図 — セクタマップでリアルタイム可視化・浄化率（Purity）数値表示
メタデータ自動取得 — MusicBrainz / GnuDB（freedb後継）/ iTunes の三段階検索
アルバムアート自動取得 — Cover Art Archive / iTunes から自動ダウンロード・FLACタグ埋め込み
FLAC / WAV 出力 — 日本語ファイル名・タグ完全対応
複数枚組CD対応 — Disc1 / Disc2 ... を自動振り分け
AccurateRip ベリファイ — リッピング品質を確認
CUE / BIN / WAV 対応 — CDドライブなしでもリッピング可能
5ステップウィザード — Source → Output → Metadata → RIP → Verify
連続リッピング対応 — リセットボタンで再起動不要
---
動作要件
Windows 10 / 11（64bit）
CDドライブ（外付けUSB対応）または CUE / BIN / WAV ファイル
インターネット接続推奨（メタデータ・アルバムアート取得時）
---
インストール
Releases から `Always_CD_Ripper_Setup_1.0.0.msi` をダウンロードしてダブルクリック。
---
使い方
① Source — ソース選択
CDドライブ または CUE / BIN / WAV ファイルを選択
② Output — 出力設定
出力先フォルダとフォーマット（FLAC推奨）を選択
③ Metadata — メタデータ確認
アルバム名・アーティスト・曲名・アルバムアートを確認・編集  
複数枚組CDの場合は DISC 番号を指定
④ RIP — リッピング実行
RIP START ボタンで開始。セクタマップとログをリアルタイム表示
⑤ Verify — ベリファイ
AccurateRip DB と照合して品質を確認
---
出力フォルダ構成
```
出力先\
  └─ アーティスト名\
       └─ (年) アルバム名\
             ├─ 01. 曲名.flac
             ├─ 02. 曲名.flac
             └─ cover.jpg
```
複数枚組の場合：
```
出力先\
  └─ アーティスト名\
       └─ (年) アルバム名\
             ├─ Disc1\
             │    ├─ 01. 曲名.flac
             │    └─ cover.jpg
             └─ Disc2\
                  └─ 01. 曲名.flac
```
---
ビルド方法
必要環境
Visual Studio 2022
Qt 6.x (msvc2022_64)
vcpkg
依存ライブラリ
```
C:\vcpkg\vcpkg install libflac taglib --triplet x64-windows
```
ビルド
```
cmake -B build -S . -G "Visual Studio 17 2022" -A x64 ^
  -DCMAKE_PREFIX_PATH=C:/Qt/6.x.x/msvc2022_64 ^
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```
---
Always シリーズ
ソフト	説明
Always Player	高音質オーディオプレーヤー
Always CD Ripper	高精度CDリッパー
---
ライセンス
GPL-3.0 © 2026 YOUICHI SAIJO
---
作者
YOUICHI SAIJO  
GitHub: @YOUICHI-del  
Homepage: always-player.sakuraweb.com
