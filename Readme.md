# Easygram

It is TUI telegram client created for programmers. I want to add here much feutures.

## Prerequirements

I using free libraries in my code. If there are some errors you can change CMakeLists.txt, because it is in develop

### TDLib

``` bash
cd libs/
git clone https://github.com/tdlib/td
cd td/
mkdir build
cd build/
cmake -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Release ..
make
make install
```

## Building

To build just write

``` bash
make bld
```

in root path.

## Running

To run builded just write

``` bash
make run
```
