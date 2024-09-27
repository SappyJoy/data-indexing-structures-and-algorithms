
## Clangd

Build project for the first time, to add create build symbols

```sh
mkdir build
cmake -S . -B build/ -DCMAKE_BUILD_TYPE=Release
cmake --build build/ -j <number of jobs>
```

