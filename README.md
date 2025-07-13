# libmetal_apu_control

このリポジトリは、**AMD Xilinx Kria SOM（K26）** プラットフォーム上の **APU（Application Processing Unit）** で動作する Linux ユーザー空間アプリケーションです。
[libmetal](https://github.com/OpenAMP/libmetal) ライブラリを使用して、**AMP（非対称マルチプロセッシング）構成**における APU と RPU（Real-time Processing Unit）間の通信や共有メモリアクセスの制御を行うデモプログラムです。

---

## 📁 ディレクトリ構成

.
├── libmetal/ # libmetal のソースコード（ビルド／インストール用）
└── libmetal_apu_control/ # APU 側アプリケーションのソースコード

- `libmetal_apu_control.c`：APU側のメインアプリケーション
- `system/linux/xlnx/zynqmp_amp_demo/sys_init.c`：XilinxのAMPデモをベースにした初期化コード
- `Makefile`：libmetal を静的または動的にリンクするためのビルド設定

---

## ⚙️ ビルド手順

### オプション①：WSL やホスト Ubuntu 上でビルドする場合（libmetal ソースを使用）

```bash
cd libmetal_apu_control
make
../libmetal/build/lib/libmetal.a を静的リンクしてビルドします。
```

### オプション②：Kria APU 上でネイティブビルドする場合

```bash
# （必要に応じて）libmetal をインストール
sudo apt install libmetal-dev

# Makefile を以下のように実行
make INC=-I/usr/include LIBS="-lmetal"
```

---

## 実行環境の要件
- APU 側で Linux（例：Petalinux や Ubuntu）が動作していること
- RPU 側で対応する OpenAMP または libmetal ベースのファームウェアが実行されていること
- デバイスツリーにおける共有メモリ（shm）・IPI 設定が正しく構成されていること

## 補足情報
- 本アプリケーションは Xilinx 提供の AMP デモ（zynqmp_amp_demo）を参考に構築されています
- APU と RPU 間での共有メモリ通信や IPI 割り込み通知の動作確認を目的とした構成です
- Petalinux 2022.2 環境での動作を確認済みです

---