## db-splitter

> :exclamation: It is only recommended to build and use on the Linux platform

a cpp cli tool to build, split, and distribute CDB of [google git-repo](https://gerrit.googlesource.com/git-repo) managed projects

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

### Usage

-   General

```shell
  ./build/Release/bin/db-splitter COMMAND {OPTIONS}

    a program to split CDB of repo managed C/C++ projects

  OPTIONS:

      commands
        split                             command to split CDB
      arguments
        -h, --help                        display global help menu
```

-   Split

```shell
  ./build/Release/bin/db-splitter split {OPTIONS} [destination]

    command to split CDB

  OPTIONS:

      repo manifests file group
        -x[manifests...],
        --xml=[manifests...]              repo manifest files
        -d[manifests directory],
        --dir=[manifests directory]       repo manifests directory
      -b[AOSP base directory],
      --base=[AOSP base directory]      AOSP base directory
      -c[path], --cdb=[path]            path of compile_commands.json to be
                                        splitted
      destination                       destination folder to store splitted CDB
      -h, --help                        display global help menu
      "--" can be used to terminate flag options and force all following
      arguments to be treated as positional options


```

**说明**
目前 db-splitter 只有一个子命令`split`，用以将针对整个 repo 管理的项目生成的 compile_commands.json 分割到不同的子项目。该选项有如下几个不同的选项：

-   `-x` 指定不同的 repo manifests（repo 通过 repo manifests 管理不同的 git 项目，不同子项目的路径-名字映射关系写在了项目根目录.repo 目录下的 manifest 文件里，如果有多个 manifest 文件，则需要使用多个-x 选项）
-   `-d` 指定一个 manifests 目录，db-splitter 会自动查找该目录下的所有 manifest 文件。
-   `-b` repo 管理的超项目基础路径（用来自动修正 compile_commands.json 中的路径映射关系）
-   `-c` 指定需要切割的 compile_commands.json 的位置
-   `destination` 指定一个切分后的 compile_commands.json 需要写入的目录

一个基础的使用例子如下：

```shell
./build/Release/bin/db-splitter split -d ~/Desktop/mergebot/aosp/.repo/manifests/ -c ~/Desktop/compile_commands.json -b ~/Desktop/mergebot/aosp  ~/Desktop/compile_commands
```

上面是一个基础的使用的例子，通过-d 指定了 manifests 的目录（如果 manifest 文件分散在多个位置需要通过`-x`选项分别指定），通过`-c`指定了待分割 compile_commands.json 的路径，通过`-b`指定了 AOSP 项目的基础路径，通过 position argument 指定了需要将输出的 compile_commands.json 输出到的目录。

一次成功的运行会显示类似如下的输出：

```shell
parsing split subcommand...
manifest files vector size: 1, first element: /home/whalien/Desktop/mergebot/aosp/.repo/manifests/default.xml
use aosp base path at /home/whalien/Desktop/mergebot/aosp
cdb_file path: /home/whalien/Desktop/compile_commands.json
dest_folder path: /home/whalien/Desktop/compile_commands

on unix platform, use mmap `MAP_PRIVATE` to do inplace json parse
it takes 512 ms to parse cdb
size of compile commands array is 60377
it takes 3374 ms to process cdb
it takes 173 ms to write output

************** Stat **************

it takes 4079 ms to split cdb
```

并且会在 position argument 的路径里产生针对各个不同子项目的 compile_commands.json 文件。
