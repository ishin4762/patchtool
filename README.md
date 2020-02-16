# patchtool

[![MIT License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE) [![Build Status](https://travis-ci.org/ishin4762/patchtool.svg?branch=master)](https://travis-ci.org/ishin4762/patchtool)

汎用差分パッチツール。以下の事ができます

* ディレクトリ単位での差分抽出
* ディレクトリへの差分適用
  * かんたんなチェックサムによる比較はおこなわれます
* bsdiff/bspatchを内部で利用(圧縮にはbzip2)
  * 元のデータの17倍のメモリを食うらしいので、ブロック分割機能を将来的に実装予定です
* CLIでの提供しかまだできておりません
* かっこいい固有名詞やロゴが欲しい

## リリース情報
最新バージョン :
* https://github.com/ishin4762/patchtool/releases/tag/v0.1.0

過去のリリースはこちら :
* https://github.com/ishin4762/patchtool/releases

## パッチの作り方
```
# Windows
CMD> patchgen <旧バージョンのDIR> <新バージョンのDIR> <出力先パッチファイル>

# mac / Linux
$ ./patchgen <旧バージョンのDIR> <新バージョンのDIR> <出力先パッチファイル>
```

## パッチの適用方法
```
# Windows
CMD> patchapply.exe <適用先DIR> <パッチファイル>

# mac / Linux
$ ./patchapply <適用先DIR> <パッチファイル>
```

## 自己解凍方式のパッチの作り方と適用方法
オプションに `-e` もしくは `--executable` を追加します。

出力先パッチファイルの名前に、`.exe` (Mac/Linuxの場合は `.out`)が付与されたファイルが出力されます。

自己解凍形式では、適用先のディレクトリを指定するだけでパッチの適用ができます。

### 例
```
# 作成
CMD> patchgen -e v1.00 v1.01 diff_patch 
CMD> dir
...
2020/02/16  17:57         1,332,008 diff_patch.exe
...

# 適用
CMD> xcopy v1.00 patch_test
CMD> diff_patch patch_test
# patch_testの中身がv1.00からv1.01に変わります
```

## Windows版自己解凍形式のパッチの署名について
そのまま作成したデータは署名がありません。
配布する場合は、下記の利用のご検討をしてみてください。

* [SignTool.exe](https://docs.microsoft.com/ja-jp/dotnet/framework/tools/signtool-exe)
* [osslsigncode](https://github.com/mtrojnar/osslsigncode)

## ビルドの方法
```
$ mkdir build && cd build
$ cmake ..
$ make
```

