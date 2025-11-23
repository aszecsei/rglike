# rglike

> a small roguelike using EnTT and ftxui

[![Code Language](https://img.shields.io/github/languages/top/aszecsei/rglike?style=for-the-badge)](https://github.com/aszecsei/rglike/search?l=c%2B%2B)
[![Code Size](https://img.shields.io/github/languages/code-size/aszecsei/rglike?style=for-the-badge)](https://github.com/aszecsei/rglike)
[![Licence](https://img.shields.io/github/license/aszecsei/rglike?style=for-the-badge)](LICENSE.md)
[![Code Quality](https://img.shields.io/codefactor/grade/github/aszecsei/rglike/main?style=for-the-badge)](https://www.codefactor.io/repository/github/aszecsei/rglike/)
[![Last Commit](https://img.shields.io/github/last-commit/aszecsei/rglike?style=for-the-badge)](https://github.com/aszecsei/rglike/commits/main)

## Getting Started

This project uses CMake as its build system, with no external dependencies.
So long as you have a C++ compiler able to handle C++20, you should be able to
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

For use with Visual Studio, open `rglike.sln` in the `build` directory.

## Third-Party Libraries

This project uses a number of third-party dependencies which should be managed by
either CMake's FetchContent system or Cargo. Specifically:

- [fmt](https://github.com/fmtlib/fmt)
- [spdlog](https://github.com/gabime/spdlog)
- [CLI11](https://github.com/CLIUtils/CLI11)
- [ftxui](https://github.com/ArthurSonzogni/ftxui)
- [EnTT](https://github.com/skypjack/entt)
- [Lua 5.4](https://github.com/lua/lua) (using the CMake bundle from [marovira](https://github.com/marovira/lua))
  - [sol2](https://github.com/ThePhD/sol2)

Please consider supporting the maintainers of these libraries!

## Known Issues

### Visual Studio

When opening the `.sln`, you may find the build process not working with error `Unable to start program '{Directory}\Debug\ALL_BUILD'. Access is denied.`.  This is because the default startup project is `ALL_BUILD`, but you actually want to choose `rglike`. Follow `Local Windows Debugger (Dropdown) > Configure Startup Projects... -> Single Startup Project > rglike`.

Now that the project is running, you may find data isn't properly populating. In some versions of Visual Studio, it will run from the working directory rather than the output directory. To solve this, enter the `rglike` project properties (Right click the `rglike` project in `Solution Explorer` and then select `Properties`.) Once there: `Configuration Properties > Build Events > Post-Build Event -> Command Line (Drop Down) > Edit`. In the resulting editor, under the `setlocal` line, then duplicate the `cmake` command line. In the duplicated command, remove `Debug` in the path. 