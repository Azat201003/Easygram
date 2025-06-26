# Easygram
It is TUI telegram client created for programmers. I want to add here much feutures.

### Prerequirements
I using free libraries in my code

#### FTXUI
Installing release of [FTXUI](https://github.com/ArthurSonzogni/FTXUI/releases) v6.1 and latest. Move it in libs folder

#### TDlib
```
cd libs/
git clone https://github.com/tdlib/td
cd td/
mkdir build
cd build/
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -- -j $(nproc)
cmake --install . --prefix ../install
make
```

### Building
To build just write 
```
make bld
```
in root path.

### Running
To run builded just write
```
make run
```
