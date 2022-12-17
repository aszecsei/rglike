# rglike

> a small roguelike using EnTT and ftxui

[![Code Language](https://img.shields.io/github/languages/top/aszecsei/rglike?style=for-the-badge)](https://github.com/aszecsei/rglike/search?l=c%2B%2B)
[![Code Size](https://img.shields.io/github/languages/code-size/aszecsei/rglike?style=for-the-badge)](https://github.com/aszecsei/rglike)
[![Licence](https://img.shields.io/github/license/aszecsei/rglike?style=for-the-badge)](LICENSE.md)
[![Code Quality](https://img.shields.io/codefactor/grade/github/aszecsei/rglike/main?style=for-the-badge)](https://www.codefactor.io/repository/github/aszecsei/rglike/)
[![Last Commit](https://img.shields.io/github/last-commit/aszecsei/rglike?style=for-the-badge)](https://github.com/aszecsei/rglike/commits/main)

## Getting Started

This project uses CMake as a build system, with no external dependencies.
So long as you have a C++ compiler able to handle C++17, you should be able to
build this project.

```shell
mkdir build && cd build
cmake ..
# if you're using linux makefiles as your cmake generator:
make
# if you're using ninja:
ninja
# etc
./game/game
```

## Third-Party Libraries

This project uses a number of third-party dependencies which should be managed by
CMake's FetchContent system. Specifically:

- [fmt](https://github.com/fmtlib/fmt)
- [spdlog](https://github.com/gabime/spdlog)
- [CLI11](https://github.com/CLIUtils/CLI11)
- [ftxui](https://github.com/ArthurSonzogni/ftxui)
- [EnTT](https://github.com/skypjack/entt)

Please consider supporting the maintainers of these libraries!