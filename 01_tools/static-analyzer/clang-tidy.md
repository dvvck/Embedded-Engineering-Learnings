# Clang-Tidy

Clang-Tidy is a static code analyzer that helps identifying potential error causes.

## Setup Clang-Tidy with CMake

Using CMake makes the setup for Clang-Tidy quite straight forward.
To ensure Clang-Tidy is not a mandatory dependency and used when building for production it will be encapsulated using an optional build flag.

### 1. Install Clang-Tidy in your environment e.g. using conda:

```zsh
conda install clang-tools
```

### 2. Create a Clang Tidy Config file

There are multiple ways to configure CLang-Tidy in a CMake setup. 
The approach that uses a separate `.clang-tidy` file is prefered due to better readability.
`
#### 2.1 .clang-tidy file containing the configuration:

`.clang-tidy`
```clang-tidy
Checks: '-modernize-use-trailing-return-type,*'
```

#### 2.2 CMake file containing the Clang-Tidy configuration:

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

Add the configuration file to the root `CMakeLists.txt`

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

### 3. (optional) Add the flag to the `.vscode/settings.json`
Adding the flag to the vscode cmake configuration enables Clang-Tidy when build the project using the vscode gui.

```json
{
    "cmake.configureSettings": { "ENABLE_CLANG_TIDY": "ON" }
}
```

### 4.  Build the project


```zsh
cmake -B build -DENABLE_CLANG_TIDY=ON
cmake --build build
```
For production just ignore the flag.

## Troubleshooting the setup

### clang tidy can´t find certain system headers

It may be needed to include the system headers explicitly to resolve missing header problems.
But that may cause a new problem since including gcc specific headers and then running clang over them will cause problems. 
Therefore gcc specific directories should be excluded.

`CMakeLists.txt`
```CMake
foreach(include ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
    if(NOT ${include} MATCHES ".*lib/gcc.*")
        list(APPEND CLANG_TIDY_CMD "--extra-arg=-isystem${include}")
    endif()
endforeach()

set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_CMD}")
```

### use of undeclared identifier '__builtin_ia32_addss' [clang-diagnostic-error]

Clang-Tidy uses the clang compiler.
If the compiler for the project is the gcc this may cause problems caused by differences in compiler specific headers.
To fix this it is important to include the clang compiler in the `CMakeLists.txt` in order to tell Clang-Tidy which headers it should use.

`CMakeLists.txt`
```CMake
find_program(CLANG_EXE NAMES "clang")
if(CLANG_EXE)
    execute_process(
        COMMAND ${CLANG_EXE} --print-resource-dir
        OUTPUT_VARIABLE CLANG_RESOURCE_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    list(APPEND CLANG_TIDY_CMD "--extra-arg=-isystem${CLANG_RESOURCE_DIR}/include")
    message(STATUS "Clang resource dir: ${CLANG_RESOURCE_DIR}")
else()
    message(WARNING "clang not found — resource headers unavailable, intrinsic errors may occur")
endif()
```

### wrong macros

Many gcc c headers use macros but they are platform specific, thats why the target platform might be necessary to declare.

`CMakeLists.txt`
```CMake
list(APPEND CLANG_TIDY_CMD "--extra-arg=--target=x86_64-pc-linux-gnu")
```

### Working example

All the above mentioned problems arose on an x86_64 CHip running Arch Linux.
The following CMake configuration fixed it for me.

`CMakeLists.txt`
```cmake
#### Add ClangTidy as an optional dependency
option(ENABLE_CLANG_TIDY "Enable clang-tidy static analysis" OFF)

if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
    find_program(CLANG_EXE NAMES "clang")

    if(CLANG_TIDY_EXE)
        message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")

        set(CLANG_TIDY_CMD
            "${CLANG_TIDY_EXE}"
            "--system-headers=0"
            "--header-filter=./.*"
            "--config-file=${CMAKE_SOURCE_DIR}/.clang-tidy"
        )

        # 1. Tell clang the exact target triple → ensures correct predefined macros
        #    (__SIZE_TYPE__, __PTRDIFF_TYPE__ etc.) so size_t resolves in global namespace
        list(APPEND CLANG_TIDY_CMD "--extra-arg=--target=x86_64-pc-linux-gnu")

        # 2. Inject clang's own resource headers FIRST
        #    These provide clang-compatible xmmintrin.h, stddef.h, etc.
        if(CLANG_EXE)
            execute_process(
                COMMAND ${CLANG_EXE} --print-resource-dir
                OUTPUT_VARIABLE CLANG_RESOURCE_DIR
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
            list(APPEND CLANG_TIDY_CMD "--extra-arg=-isystem${CLANG_RESOURCE_DIR}/include")
            message(STATUS "Clang resource dir: ${CLANG_RESOURCE_DIR}")
        else()
            message(WARNING "clang not found — resource headers unavailable, intrinsic errors may occur")
        endif()

        # 3. Add GCC's implicit includes as proper system headers, skipping lib/gcc builtins
        #    (handled by clang's resource headers above)
        foreach(include ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
            if(NOT ${include} MATCHES ".*lib/gcc.*")
                list(APPEND CLANG_TIDY_CMD "--extra-arg=-isystem${include}")
            endif()
        endforeach()

        # Do NOT use CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES — it bypasses proper
        # C/C++ header namespace handling and causes size_t to land only in std::
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_CMD}")

    else()
        message(WARNING "clang-tidy not found – static analysis disabled")
    endif()
endif()
```

## Clang-Tidy checks

Clang-Tidy checks are used to configure clang-tidy.
By default every  check is enabled. 
The checks can be configured using regex. and are being defined comma seperated in a list.
Since they are being parsed left to right make sure to increase granularity and not start with a fine coarsed check and afterwards accidently activate it again with a more general regex.

```clang-tidy
Checks: '-*,modernize-*,performance-*,bugprone-*, -modernize-use-trailing-return-type'
```

### modernize-use-trailing-return-type

This flag is used to enforce a specific C++11 syntax style which is used in some code bases that heavily rely on templates, decltype, and explicit lambda return types.
In codebases that are not excessively using the above mentioned functionalities it should be disabled.

### clang-diagnostic-unused-private-field

A private member variable is being declared but not used.

### performance-avoid-endl

`std::endl` flushes the output buffer even if it is not full.
This can lead to performance issues, therefore it is advised to use `\n` instead especially in loops.
It should only be used when immeadiate flushing is important e.g. writing errors before a system crash or realtime progress bars etc.

### modernize-pass-by-value

Passing `std::shared_ptr<>` objects in the constructor should be done using `std::move` internally.
When passing the pointer to the constructor the ref counter is increased.
Using `std::move` in the initializer list just makes the member point to the existing object and the passed parameter object will be a `nullptr` afterwards and destroyed when leaving the constructor.

```cpp
Foo::Foo(std::shared_ptr<IBar> bar) : m_bar(std::move(bar)){};
```
### performance-unnecessary-value-param

This check ensures that expensive objects like `std::vector<>`or `std::string` are not being passed by value if they are only read in the function body.
They should be passed as `const &T`instead.

### modernize-use-nodiscard

This check is very aggresive and enforces to use the C++17 `[[nodiscard]]` attribute whenever there is a const member function with
- a non void return type,
- no pointer or non-const-reference parameter,
- no template parameter,
- no mutable member variables in the class.

It is supposed to warn if a return value is being ignored.
The C++ committee only uses it if ignoring the return value will definitely cause a bug in the software.

### modernize-use-override

Inheriting from a baseclass often results in overriding vertain methods especially if they are virtual. 
This check ensures that each virtual function in a derived class uses the `override` keyword.
This makes identifying bugs cause by differing method signatures etc. easier.

The destructor is an edge case in that regard. 
Since its signature is fixed there is no real benefit regarding the ensurance of a mathing function signature and thats why it is quite a controversial topic, even in the C++ Core Guidelines.
However if the base class destuctor is being removed, using `override` in the derived class would result in a compiler error and therefore improve safety.

### performance-for-range-copy

Range based for loops should use a const reference if they are not modified instead of creating a new copy with each iteration.

```cpp
//bad
for (auto foo : bar){
    std::cout << foo << std::endl;
}

//better
for (const auto& foo : bar){
    std::cout << foo << std::endl;
}
```