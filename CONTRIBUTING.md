# How to contribute

## First rule of our club: use clang-format

Initialize `clang-format-hooks`, so you can't push with format errors

```sh
git submodule update --init
cd clang-format-hooks && ./git-pre-commit-format install
```

