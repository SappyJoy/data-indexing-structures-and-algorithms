# How to contribute

## First rule of our club: use clang-format

Initialize `clang-format-hooks`, so you can't push with format errors

```sh
git submodule update --init
cd clang-format-hooks && ./git-pre-commit-format install
```

## Clangd

Build project for the first time, to add create build symbols

```sh
mkdir build
cmake -S . -B build
cmake --build build -j <number of jobs>
```
