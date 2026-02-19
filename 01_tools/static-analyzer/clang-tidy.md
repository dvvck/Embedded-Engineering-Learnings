# Clang-Tidy

Clang-Tidy is a static code analyzer that helps identifying potential error causes.

## Setup Clang-Tidy with CMake

Using CMake makes the setup for Clang-Tidy quite straight forward.
To ensure Clang-Tidy is not a mandatory dependency and used when building for production it will be encapsulated using an optional build flag.

### 1. Install Clang-Tidy in your environment e.g. using conda:

```zsh
conda install clang-tools
```

### 2. Create a CMake file containing the Clang-Tidy configuration:

`cmake/ClangTidy.cmake`
```cmake
find_program(CLANG_TIDY_EXE NAMES "clang-tidy")

if(CLANG_TIDY_EXE)
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}"
        "-checks=-*,modernize-*,performance-*,bugprone-*"
        "-header-filter=${CMAKE_SOURCE_DIR}/.*"
    )
else()
    message(STATUS "clang-tidy NOT found – static analysis disabled")
endif()
```

### 3. Add the configuration file to the root `CMakeLists.txt`
```cmake
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_CLANG_TIDY "Enable clang-tidy static analysis" OFF)

if(ENABLE_CLANG_TIDY)
    # Fix to use system headers aswell
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})

    include(cmake/ClangTidy.cmake)
endif()
```
Clang-Tidy uses the cmake `compile_commands.json`. 
Usually then don´t contain the system headers which caused problems for me. 
Thats why they need to be included explicitly. 

### 4. (optional) Add the flag to the `.vscode/settings.json`
Adding the flag to the vscode cmake configuration enables Clang-Tidy when build the project using the vscode gui.

```json
{
    "cmake.configureSettings": { "ENABLE_CLANG_TIDY": "ON" }
}
```

### 5.  Build the project


```zsh
cmake -B build -DENABLE_CLANG_TIDY=ON
cmake --build build
```
For production just ignore the flag.

