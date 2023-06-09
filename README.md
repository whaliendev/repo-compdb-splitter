<h2 align="center"> repo-compdb-splitter</h2>
<p align="center">a golang cli tool to build, split and distribute CompDB of repo managed projects</p>

### Concepts
- [CompDB](https://clang.llvm.org/docs/JSONCompilationDatabase.html) is typically generated for C/C++ projects for 
clang-based tool to parse C/C++ sources
- [git-repo](https://gerrit.googlesource.com/git-repo) is a VCS developed by google to manage multi git controlled repo

In the Google [AOSP](https://source.android.com/) project, we use the `git-repo` tool to manage thousands of subprojects controlled by git. 
When we need to generate a `compile_commands.json` file to different C/C++ subprojects, we use the 
[compiledb](https://github.com/nickdiego/compiledb) tool to parse build logs and generate a complete `CompilationDatabase`. 
However, this complete CompilationDatabase is generated for the entire AOSP project, 
and loading this file into memory using a clang-based tool is very expensive and time-consuming. 

repo-compdb-splitter is designed to automatically parse the manifest.xml file of git-repo to build a dictionary tree, 
match and split `CompileCommand` from the complete `CompilationDatabase`, and distribute them to different C/C++ AOSP subprojects.

### Build
```shell
todo
```

### Usage
```shell
todo
```

### License

[LPL-2.1](LICENSE)

<center>Copyright © 2023 Hwa</center>

---

<p align="center"><b>If you like my project, feel free to give my repo a star~ :star: :arrow_up:. </b></p>
