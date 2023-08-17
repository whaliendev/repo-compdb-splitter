## db-splitter

a cpp cli tool to build, split, and distribute CDB of repo managed projects

### Build

#### Base Environment

-   GCC >= 9, db-splitter requires a C++17 compliant compiler for building (any compiler supports stdc++17 is ok)
-   Python >= 3.6, we use Python and conan2 for managing third-party dependencies
-   CMake >= 3.21, CMake is the de factor standard build system generator for C++

#### External Dependencies

We use conan2 to manage external dependencies. Before using conan2 to manage external dependencies, we need to install and initialize it.

-   Installation

```shell
pip install conan
```

-   Initialization
    Generate a default conan2 profile first:

```shell
conan profile detect --force
```

Then, modify the C++ compiler standard to stdc++17 in the default profile of conan2:

```shell
sed -i 's/compiler.cppstd=[^ ]*/compiler.cppstd=17/' ~/.conan2/profiles/default
```

Once these steps are completed, we can use conan to manage the project's dependencies.

We can install all external libs using the following command:

```shell
cd <db-splitter's base directory>
conan install . --build=missing -r=conancenter -s build_type=[Release | RelWithDebInfo | Debug | MinSizeRel]
```

> P.S.: When we use angle brackets in markdown code block, we mean you need to substitute them with the content within; when we use square brackets, you can choose an item from the content within the brackets to concatenate into a complete command.

#### Build

```shell
cmake --preset [conan-debug | conan-release]
cmake --build build/[Debug | Release]
```
