# Magnum Moon Lander

A prototype example of a modern Lunar Lander game in C++ using the
[Magnum Graphics Engine](https://github.com/mosra/magnum) and [Box2D](https://box2d.org/).

![Alt text](doc/moonlander-1.png "Screenshot")

## Linux Instructions

### Fedora / Rocky Linux
There are Corrade, Magnum, and Box2D packages in the [TEK Federation RPM Repository](https://rpm.tekfed.org/). 

## Windows Instructions
**Install Corrade and Magnum from sources**

Install [MSYS2](https://www.msys2.org/) and open a **UCRT64** (or **CLANG64**) shell and follow
[instructions](https://doc.magnum.graphics/magnum/building-integration.html#building-integration-packages-msys)
to build the latest Magnum and Corrade packages.

**Install Box2D from official repository**
```
pacman -S mingw-w64-ucrt-x86_64-box2d
# or ...
pacman -S mingw-w64-clang-x86_64-box2d
```
