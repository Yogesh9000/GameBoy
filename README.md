# NoobBoy

A simple Game Boy emulator written in C++.

## Building NoobBoy

### Prerequisites
- Conan 2 (package manager)  
  Install from: https://docs.conan.io/2/installation.html
- A C++ compiler (GCC, Clang, MSVC, etc.)
- CMake
- A CMake generator (Make, Ninja, Visual Studio, etc.)

> [!NOTE]
> On Windows, you can use a package manager like https://scoop.sh/ to install all prerequisites.

---

### Build Instructions

1. Clone the repository:
```sh
git clone git@github.com:Yogesh9000/GameBoy.git
```
2. Navigate to the project directory:
```sh
cd GameBoy
```
3. Install dependencies using Conan:
```sh
conan install . --output-folder=build --build=missing -s build_type=Release
```
4. Configure the project with CMake:
```sh
cd build
cmake .. -G "Visual Studio 18 2026" -DCMAKE_TOOLCHAIN_FILE="conan_toolchain.cmake"
```
5. Build the project:
```sh
cmake --build . --config=Release
```

---

> [!NOTE]
> - The steps above build the project in Release mode.  
> - For a Debug build, replace Release with Debug in the commands.  
> - The example uses the Visual Studio 18 2026 generator, but you can use others such as Ninja or Make.
