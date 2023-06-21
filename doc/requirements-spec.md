## 需求说明

[TOC]

### 背景

#### Compdb

基于C++抽象语法树的工具通常需要知道怎么parse一个translation unit的全部信息。比如clang based的工具clangd(
一个C++语言服务器)、基于clang LibTooling库写的C++编译前端frontend action、以及一些其他的工具。
总之，由于C++和C语言源文件非自包含、脆弱的包管理机制等历史包袱，社区对C++和C语言做静态分析时希望可以得到来自开发者的一些提示，主要是：

+ 编译使用的工具，如`gcc` or `g++`, `clang` or `clang++`。
+ 编译使用的语言标准，如`--std=c++11, --std=c++17`等。
+ 引用的库文件中头文件的位置，如gcc, clang, g++, clang++编译时指定的`-I`参数。
+ ...

在这些需求下，有了[CompDB](https://clang.llvm.org/docs/JSONCompilationDatabase.html)
这个概念。这个概念指的通常是项目的构建目录下的一个叫`compile_commands.json`的文件。它通常长这个样子：

```json
[
  {
    "directory": "/home/whalien/Desktop/rocksdb/build",
    "command": "/usr/bin/c++ -DGFLAGS=1 -DGFLAGS_IS_A_DLL=0 -DOS_LINUX -DROCKSDB_AUXV_GETAUXVAL_PRESENT -DROCKSDB_FALLOCATE_PRESENT -DROCKSDB_LIB_IO_POSIX -DROCKSDB_MALLOC_USABLE_SIZE -DROCKSDB_NO_DYNAMIC_EXTENSION -DROCKSDB_PLATFORM_POSIX -DROCKSDB_PTHREAD_ADAPTIVE_MUTEX -DROCKSDB_RANGESYNC_PRESENT -DROCKSDB_SCHED_GETCPU_PRESENT -I/home/whalien/Desktop/rocksdb -I/home/whalien/Desktop/rocksdb/include -isystem /home/whalien/Desktop/rocksdb/third-party/gtest-1.8.1/fused-src  -W -Wextra -Wall -pthread -Wsign-compare -Wshadow -Wno-unused-parameter -Wno-unused-variable -Woverloaded-virtual -Wnon-virtual-dtor -Wno-missing-field-initializers -Wno-strict-aliasing -Wno-invalid-offsetof -march=native -Werror -fno-builtin-memcmp -g -DROCKSDB_USE_RTTI -std=gnu++17 -o CMakeFiles/rocksdb.dir/cache/cache.cc.o -c /home/whalien/Desktop/rocksdb/cache/cache.cc",
    "file": "/home/whalien/Desktop/rocksdb/cache/cache.cc",
    "output": "CMakeFiles/rocksdb.dir/cache/cache.cc.o"
  },
  {
    "directory": "/home/whalien/Desktop/rocksdb/build",
    "command": "/usr/bin/c++ -DGFLAGS=1 -DGFLAGS_IS_A_DLL=0 -DOS_LINUX -DROCKSDB_AUXV_GETAUXVAL_PRESENT -DROCKSDB_FALLOCATE_PRESENT -DROCKSDB_LIB_IO_POSIX -DROCKSDB_MALLOC_USABLE_SIZE -DROCKSDB_NO_DYNAMIC_EXTENSION -DROCKSDB_PLATFORM_POSIX -DROCKSDB_PTHREAD_ADAPTIVE_MUTEX -DROCKSDB_RANGESYNC_PRESENT -DROCKSDB_SCHED_GETCPU_PRESENT -I/home/whalien/Desktop/rocksdb -I/home/whalien/Desktop/rocksdb/include -isystem /home/whalien/Desktop/rocksdb/third-party/gtest-1.8.1/fused-src  -W -Wextra -Wall -pthread -Wsign-compare -Wshadow -Wno-unused-parameter -Wno-unused-variable -Woverloaded-virtual -Wnon-virtual-dtor -Wno-missing-field-initializers -Wno-strict-aliasing -Wno-invalid-offsetof -march=native -Werror -fno-builtin-memcmp -g -DROCKSDB_USE_RTTI -std=gnu++17 -o CMakeFiles/rocksdb.dir/cache/cache_entry_roles.cc.o -c /home/whalien/Desktop/rocksdb/cache/cache_entry_roles.cc",
    "file": "/home/whalien/Desktop/rocksdb/cache/cache_entry_roles.cc",
    "output": "CMakeFiles/rocksdb.dir/cache/cache_entry_roles.cc.o"
  },
  {
    "directory": "/home/whalien/Desktop/rocksdb/build",
    "command": "/usr/bin/c++ -DGFLAGS=1 -DGFLAGS_IS_A_DLL=0 -DOS_LINUX -DROCKSDB_AUXV_GETAUXVAL_PRESENT -DROCKSDB_FALLOCATE_PRESENT -DROCKSDB_LIB_IO_POSIX -DROCKSDB_MALLOC_USABLE_SIZE -DROCKSDB_NO_DYNAMIC_EXTENSION -DROCKSDB_PLATFORM_POSIX -DROCKSDB_PTHREAD_ADAPTIVE_MUTEX -DROCKSDB_RANGESYNC_PRESENT -DROCKSDB_SCHED_GETCPU_PRESENT -I/home/whalien/Desktop/rocksdb -I/home/whalien/Desktop/rocksdb/include -isystem /home/whalien/Desktop/rocksdb/third-party/gtest-1.8.1/fused-src  -W -Wextra -Wall -pthread -Wsign-compare -Wshadow -Wno-unused-parameter -Wno-unused-variable -Woverloaded-virtual -Wnon-virtual-dtor -Wno-missing-field-initializers -Wno-strict-aliasing -Wno-invalid-offsetof -march=native -Werror -fno-builtin-memcmp -g -DROCKSDB_USE_RTTI -std=gnu++17 -o CMakeFiles/rocksdb.dir/cache/cache_key.cc.o -c /home/whalien/Desktop/rocksdb/cache/cache_key.cc",
    "file": "/home/whalien/Desktop/rocksdb/cache/cache_key.cc",
    "output": "CMakeFiles/rocksdb.dir/cache/cache_key.cc.o"
  },
  ...
]
```

整个文件是一个Array<CompileCommand>。也就是说，是一个填充`CompileCommand`的数组。每个compile command有四个字段，具体的内容见LLVM的data
schema说明和项目根目录下的`model/compdb.go`文件中的data model。
CompDB常见的生成方式： https://sarcasm.github.io/notes/dev/compilation-database.html
其中比较重要的，我们关注的有下面几个：

+ cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON, 由于cmake已成为C++开源的事实标准，所以很多开源项目我们使用这种方式构建
+ compiledb 一个python写的生成C++ compile db的工具，可以不build直接生成，也可以解析项目的构建日志进行compdb生成。
+ bear 这个工具可以拦截C++/C的编译过程，在其他工具无法不构建生成compdb的情况下，bear往往是最后的考虑。

#### AOSP

AOSP (Android Open Source Project, abbrev as AOSP),
安卓开源项目，是由谷歌主导、众多移动通信公司、社区开源共建的一个基于Linux内核的移动端操作系统。该项目由谷歌开发的多仓版本控制工具[repo](https://gerrit.googlesource.com/git-repo)
调度git进行管理。
AOSP项目下由工具repo管理着1000多个子项目，如根目录下：

```shell
.
├── art               android runtime
├── assets
├── bionic            
├── bootable
├── cts
├── dalvik
├── developers
├── development
├── device            device specified code
├── external
├── frameworks        安卓向上提供的frameworks
├── hardware          硬件驱动
├── kernel
├── libcore
├── libnativehelper
├── out               构建结果输出目录
├── packages
├── pdk
├── platform_testing
├── prebuilts
├── sdk
├── system
├── test
├── toolchain
├── tools
└── WORKSPACE -> build/bazel/bazel.WORKSPACE
```

并且采用如下的方式进行构建： https://source.android.com/docs/setup/build/building
（主要就是source build/envsetup.sh后mm从根开始构建和mma构建当前子项目和其依赖）

在安卓中，我们也可以采用如下方式进行CompDB生成： https://android.googlesource.com/platform/build/soong/+/HEAD/docs/compdb.md

### 问题

考虑和OPPO交付项目对可用性要求比较高，我们计划采用[compiledb](https://github.com/nickdiego/compiledb)
这个工具通过解析项目中已存在的构建日志进行compdb的生成。
主要是其中的：

```shell
compiledb --parse build-log.txt
```

在AOSP项目中，即在根目录下采用`mm`进行全量编译后 采用`compiledb --parse out/verbose.log`
。但是通过这种方式造成的问题是，生成的`compile_commands.json`
是针对整个AOSP项目的，但我们要做程序分析的是单个git项目，为单个项目加载整个compile_commands.json进内存显然是非常不划算和低效的。所以我们需要开发这个小工具进行compdb的匹配，拆分、分发和清理。

### 具体思路

1. repo这个工具在项目的根目录下也有个隐藏目录叫： `.repo`，下面的`manifest.xml`
   文件中有repo管理的所有项目的元信息，所以我们可以解析这个文件来获取这个超级项目下所有的用git管理的子项目。一个`manifest.xml`
   的例子是：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<manifest>

    <remote name="aosp"
            fetch=".."
            review="https://android-review.googlesource.com/"/>
    <default revision="master"
             remote="aosp"
             sync-j="4"/>

    <manifest-server url="http://android-smartsync.corp.google.com/android.googlesource.com/manifestserver"/>

    <superproject name="platform/superproject" remote="aosp"/>
    <contactinfo bugurl="go/repo-bug"/>

    <project path="build/make" name="platform/build" groups="pdk">
        <linkfile src="CleanSpec.mk" dest="build/CleanSpec.mk"/>
        <linkfile src="buildspec.mk.default" dest="build/buildspec.mk.default"/>
        <linkfile src="core" dest="build/core"/>
        <linkfile src="envsetup.sh" dest="build/envsetup.sh"/>
        <linkfile src="target" dest="build/target"/>
        <linkfile src="tools" dest="build/tools"/>
    </project>
    <project path="build/orchestrator" name="platform/build/orchestrator" groups="pdk"/>
    <project path="build/bazel" name="platform/build/bazel" groups="pdk">
        <linkfile src="bazel.WORKSPACE" dest="WORKSPACE"/>
        <linkfile src="bazel.BUILD" dest="BUILD"/>
    </project>
    <project path="build/bazel_common_rules" name="platform/build/bazel_common_rules" groups="pdk"/>
    <project path="build/blueprint" name="platform/build/blueprint" groups="pdk,tradefed"/>
    <project path="build/pesto" name="platform/build/pesto" groups="pdk"/>
    <project path="build/soong" name="platform/build/soong" groups="pdk,tradefed">
        <linkfile src="root.bp" dest="Android.bp"/>
        <linkfile src="bootstrap.bash" dest="bootstrap.bash"/>
    </project>
    <project path="art" name="platform/art" groups="pdk"/>
    <project path="bionic" name="platform/bionic" groups="pdk"/>
    <project path="bootable/recovery" name="platform/bootable/recovery" groups="pdk"/>
    ...
</manifest>
```

可以看到每个项目有个path字段（相对路径），name字段（名称）等。我们可以考虑采用正则表达式(RE)解析这个文件获取这些project
item，然后对相对路径中的segment构建Trie Tree。

2. 下面是生成的compile_commands.json的部分例子：

```json
[
  {
    "directory": "/home/hwa/Documents/mb/aosp",
    "arguments": [
      "prebuilts/clang/host/linux-x86/clang-r475365b/bin/clang",
      "-D__ANDROID_VNDK__",
      "-D__ANDROID_VENDOR__",
      "-mthumb",
      "-Os",
      "-fomit-frame-pointer",
      "-DANDROID",
      "-fmessage-length=0",
      "-W",
      "-Wall",
      "-Wno-unused",
      "-Winit-self",
      "-Wpointer-arith",
      "-Wunreachable-code-loop-increment",
      "-no-canonical-prefixes",
      "-DNDEBUG",
      "-UDEBUG",
      "-fno-exceptions",
      "-Wno-multichar",
      "-O2",
      "-g",
      "-fdebug-default-version=5",
      "-fno-strict-aliasing",
      "-Werror=date-time",
      "-Werror=pragma-pack",
      "-Werror=pragma-pack-suspicious-include",
      "-Werror=string-plus-int",
      "-Werror=unreachable-code-loop-increment",
      "-Wno-error=deprecated-declarations",
      "-D__compiler_offsetof=__builtin_offsetof",
      "-faddrsig",
      "-fcommon",
      "-Werror=int-conversion",
      "-Wno-reserved-id-macro",
      "-fcolor-diagnostics",
      "-Wno-sign-compare",
      "-Wno-inconsistent-missing-override",
      "-Wno-c99-designator",
      "-Wno-gnu-designator",
      "-Wno-gnu-folding-constant",
      "-Wunguarded-availability",
      "-D__ANDROID_UNAVAILABLE_SYMBOLS_ARE_WEAK__",
      "-ffp-contract=off",
      "-fdebug-prefix-map=/proc/self/cwd=",
      "-ftrivial-auto-var-init=zero",
      "-enable-trivial-auto-var-init-zero-knowing-it-will-be-removed-from-clang",
      "-Wno-unused-command-line-argument",
      "-ffunction-sections",
      "-fdata-sections",
      "-fno-short-enums",
      "-funwind-tables",
      "-fstack-protector-strong",
      "-Wa,--noexecstack",
      "-D_FORTIFY_SOURCE=2",
      "-Wstrict-aliasing=2",
      "-Werror=return-type",
      "-Werror=non-virtual-dtor",
      "-Werror=address",
      "-Werror=sequence-point",
      "-Werror=format-security",
      "-nostdlibinc",
      "-fdebug-info-for-profiling",
      "-msoft-float",
      "-march=armv7-a",
      "-mfloat-abi=softfp",
      "-mfpu=neon",
      "-Ihardware/libhardware/modules/audio",
      "-Ihardware/libhardware/include",
      "-Isystem/media/audio/include",
      "-Isystem/core/libcutils/include_outside_system",
      "-Isystem/core/libsystem/include",
      "-Ipackages/modules/Bluetooth/system/types",
      "-Iexternal/libcxxabi/include",
      "-Isystem/logging/liblog/include_vndk",
      "-Iexternal/libcxx/include",
      "-isystem",
      "out/soong/.intermediates/bionic/libc/libc/android_vendor.UpsideDownCake_arm_armv7-a-neon_shared/gen/include",
      "-isystem",
      "bionic/libc/kernel/uapi/asm-arm",
      "-isystem",
      "bionic/libc/kernel/uapi",
      "-isystem",
      "bionic/libc/kernel/android/scsi",
      "-isystem",
      "bionic/libc/kernel/android/uapi",
      "-target",
      "armv7a-linux-androideabi10000",
      "-DANDROID_STRICT",
      "-fPIC",
      "-Wall",
      "-Werror",
      "-Wno-unused-parameter",
      "-DDO_NOT_CHECK_MANUAL_BINDER_INTERFACES",
      "-std=gnu11",
      "hardware/libhardware/modules/audio/audio_policy.c"
    ],
    "file": "hardware/libhardware/modules/audio/audio_policy.c"
  },
  {
    "directory": "/home/hwa/Documents/mb/aosp",
    "arguments": [
      "prebuilts/clang/host/linux-x86/clang-r475365b/bin/clang++",
      "-DUSE_MINGW",
      "-DWIN32_LEAN_AND_MEAN",
      "-Wno-unused-parameter",
      "-D__STDC_FORMAT_MACROS",
      "-D__STDC_CONSTANT_MACROS",
      "-D__USE_MINGW_ANSI_STDIO=1",
      "-D_WIN32_WINNT=0x0601",
      "-DWINVER=0x0601",
      "-D_FILE_OFFSET_BITS=64",
      "-mno-ms-bitfields",
      "--sysroot",
      "prebuilts/gcc/linux-x86/host/x86_64-w64-mingw32-4.8/x86_64-w64-mingw32",
      "-m32",
      "-DANDROID",
      "-fmessage-length=0",
      "-W",
      "-Wall",
      "-Wno-unused",
      "-Winit-self",
      "-Wpointer-arith",
      "-Wunreachable-code-loop-increment",
      "-no-canonical-prefixes",
      "-DNDEBUG",
      "-UDEBUG",
      "-fno-exceptions",
      "-Wno-multichar",
      "-O2",
      "-g",
      "-fdebug-default-version=5",
      "-fno-strict-aliasing",
      "-Werror=date-time",
      "-Werror=pragma-pack",
      "-Werror=pragma-pack-suspicious-include",
      "-Werror=string-plus-int",
      "-Werror=unreachable-code-loop-increment",
      "-Wno-error=deprecated-declarations",
      "-D__compiler_offsetof=__builtin_offsetof",
      "-faddrsig",
      "-fcommon",
      "-Werror=int-conversion",
      "-Wno-reserved-id-macro",
      "-fcolor-diagnostics",
      "-Wno-sign-compare",
      "-Wno-inconsistent-missing-override",
      "-Wno-c99-designator",
      "-Wno-gnu-designator",
      "-Wno-gnu-folding-constant",
      "-Wunguarded-availability",
      "-D__ANDROID_UNAVAILABLE_SYMBOLS_ARE_WEAK__",
      "-ffp-contract=off",
      "-fdebug-prefix-map=/proc/self/cwd=",
      "-ftrivial-auto-var-init=zero",
      "-enable-trivial-auto-var-init-zero-knowing-it-will-be-removed-from-clang",
      "-Wno-unused-command-line-argument",
      "-Bprebuilts/gcc/linux-x86/host/x86_64-w64-mingw32-4.8/x86_64-w64-mingw32/bin",
      "-Ipackages/modules/adb",
      "-Ibuild/soong/cc/libbuildversion/include",
      "-Ipackages/modules/adb/crypto/include",
      "-Ipackages/modules/adb/pairing_connection/include",
      "-Iexternal/protobuf/src",
      "-Iout/soong/.intermediates/packages/modules/adb/proto/libadb_protos/windows_x86_static/gen/proto/packages/modules/adb/proto",
      "-Iout/soong/.intermediates/packages/modules/adb/proto/libadb_protos/windows_x86_static/gen/proto",
      "-Ipackages/modules/adb/tls/include",
      "-Isystem/libbase/include",
      "-Iexternal/fmtlib/include",
      "-Iexternal/boringssl/src/include",
      "-Isystem/core/libcrypto_utils/include",
      "-Isystem/core/libcutils/include",
      "-Isystem/core/libprocessgroup/include",
      "-Isystem/core/diagnose_usb/include",
      "-Isystem/logging/liblog/include",
      "-Iexternal/mdnsresponder/mDNSShared",
      "-Iexternal/openscreen",
      "-Iexternal/libusb/include",
      "-Isystem/core/libutils/include",
      "-Isystem/core/libsystem/include",
      "-Iexternal/libcxx/include",
      "-Iexternal/libcxxabi/include",
      "-Idevelopment/host/windows/usb/api",
      "-Iout/soong/.intermediates/development/sdk/platform_tools_version/gen",
      "-target",
      "i686-windows-gnu",
      "-DANDROID_STRICT",
      "-Wall",
      "-Wextra",
      "-Werror",
      "-Wexit-time-destructors",
      "-Wno-non-virtual-dtor",
      "-Wno-unused-parameter",
      "-Wno-missing-field-initializers",
      "-Wthread-safety",
      "-Wvla",
      "-DADB_HOST=1",
      "-DANDROID_BASE_UNIQUE_FD_DISABLE_IMPLICIT_CONVERSION=1",
      "-DUNICODE=1",
      "-D_UNICODE=1",
      "-D_GNU_SOURCE",
      "-D_POSIX_SOURCE",
      "-Wno-ignored-attributes",
      "-Wno-thread-safety",
      "-Wsign-promo",
      "-Wimplicit-fallthrough",
      "-D_LIBCPP_ENABLE_THREAD_SAFETY_ANNOTATIONS",
      "-Wno-gnu-include-next",
      "-fsjlj-exceptions",
      "-std=gnu++2a",
      "-nostdinc++",
      "-D_LIBCPP_DISABLE_VISIBILITY_ANNOTATIONS",
      "-D_LIBCXXABI_DISABLE_VISIBILITY_ANNOTATIONS",
      "-D_LIBCPP_HAS_THREAD_API_WIN32",
      "-Isystem/core/include",
      "-Isystem/logging/liblog/include",
      "-Isystem/media/audio/include",
      "-Ihardware/libhardware/include",
      "-Ihardware/libhardware_legacy/include",
      "-Ihardware/ril/include",
      "-Iframeworks/native/include",
      "-Iframeworks/native/opengl/include",
      "-Iframeworks/av/include",
      "-isystem",
      "prebuilts/gcc/linux-x86/host/x86_64-w64-mingw32-4.8/x86_64-w64-mingw32/include",
      "packages/modules/adb/sysdeps/win32/stat.cpp"
    ],
    "file": "packages/modules/adb/sysdeps/win32/stat.cpp"
  },
```

可以看到有如下的几个字段：

- directory: 编译时所在目录
- file: 相对于directory的相对路径
- arguments (optional): 编译命令数组
- command (optional): 编译命令
  我们可以提取file字段中的这个相对路径，在第一步构建的Trie Tree中进行查找，将某个Compile Command分配到某个项目的
  `build/compile_commands.json`文件里。

### 具体需求

1. 要求解析的输入manifests.xml可以有多个，或者为某个目录下的xml文件。相同的项目后面文件中的覆盖前面文件中的。
2. 快。可以看实现难度压榨多核性能。
3. 解析manifests.xml，匹配manifests.xml中的子项目，拆分compile
   commands，分发到不同的子项目中去。每次独立运行如果目标路径已有目标文件，覆盖该文件，并打log和清理运行后写入的所有compile_commands.json
   （即使运行失败，我们也要能清理已经产生的compile_commands.json和对消除子仓库的侵入性）。
4. 运行日志清晰。

