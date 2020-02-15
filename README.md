# patchtool

[![MIT License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE) [![Build Status](https://travis-ci.org/ishin4762/patchtool.svg?branch=master)](https://travis-ci.org/ishin4762/patchtool)

汎用差分パッチツール。以下の事ができます

* ディレクトリ単位での差分抽出
* ディレクトリへの差分適用
  * かんたんなチェックサムによる比較はおこなわれます
* bsdiff/bspatchを内部で利用(圧縮にはbzip2)
  * ちなみに元のデータの17倍のメモリを食うらしいです。bsdiff/bspatch以外の差分抽出も検討した方がよさそう

## リリース情報
最新バージョン : ***

過去のリリースはこちら:
https://github.com/ishin4762/patchtool/releases

## パッチの作り方(macOS, UNIX/Linux)
```
$ ./patchgen <旧バージョンのDIR> <新バージョンのDIR> <出力先パッチファイル>
```

## パッチの適用方法(macOS, UNIX/Linux)
```
$ ./patchapply <適用先DIR> <パッチファイル>
```

## パッチの作り方(Windows)
```
CMD> patchgen.exe <旧バージョンのDIR> <新バージョンのDIR> <出力先パッチファイル>
```

## パッチの適用方法(Windows)
```
CMD> patchapply.exe <適用先DIR> <パッチファイル>
```

## ビルドの方法
```
$ ./autogen.sh
$ ./configure
$ make
```

## 今後の予定

* このプログラムにかっこいい固有名詞をつける
* 自己解凍パッチの実装
* ~~CI連携(Travis CIあたり？)~~
* ~~隠しファイルの除外~~
