# patchtool

[![MIT License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

汎用差分パッチツール。以下の事ができます

* ディレクトリ単位での差分抽出
* ディレクトリへの差分適用
  * かんたんなチェックサムによる比較はおこなわれます
* データの圧縮(現段階ではbzip2のみ)

## パッチの作り方(macOS, UNIX/Linux)
```
# 非圧縮
$ ./patchgen <旧バージョンのDIR> <新バージョンのDIR> <出力先パッチファイル>

# bzip2圧縮
$ ./patchgen -c bzip2 <旧バージョンのDIR> <新バージョンのDIR> <出力先パッチファイル>
```

## パッチの適用方法(macOS, UNIX/Linux)
```
$ ./patchapply <適用先DIR> <パッチファイル>
```

## パッチの作り方(Windows)
```
# 非圧縮
CMD> patchgen.exe <旧バージョンのDIR> <新バージョンのDIR> <出力先パッチファイル>

# bzip2圧縮
CMD> patchgen.exe -c bzip2 <旧バージョンのDIR> <新バージョンのDIR> <出力先パッチファイル>
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
* 隠しファイルの除外
