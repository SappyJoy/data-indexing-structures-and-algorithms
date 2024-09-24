## Clangd

Build project for the first time, to add create build symbols

```sh
mkdir build
conan install . --build=missing
cmake -S . -B build/ -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake
cmake --build build/ -j <number of jobs>
```

