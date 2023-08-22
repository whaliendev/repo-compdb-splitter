from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout, CMakeToolchain, CMakeDeps


class DBSplitterConan(ConanFile):
    name = "db-splitter"
    version = "1.0-alpha"
    license = "LPL-2.1"
    author = "Hwa He (hwahe.cs@gmail.com)"
    url = "https://github.com/conan-io/conan-center-index"
    description = "CDB splitter of repo managed projects"
    topics = ("CompDB", "splitter", "goole-repo")

    package_type = "application"

    settings = "os", "compiler", "build_type", "arch"

    options = {
        "fPIC": [True, False]
    }

    requires = (
        "rapidjson/cci.20220822",
        "rapidxml/1.13",
        "onetbb/2021.3.0",
        "taywee-args/6.4.6"
    )

    default_options = {
        "fPIC": True,
        "onetbb/*:shared": False,   # on Windows, shared lib cannot be linked
        "onetbb/*:fPIC": True,
        "onetbb/*:tbbmalloc": True,
        "onetbb/*:tbbproxy": True,
    }

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self, "Ninja")
        tc.generate()
        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.build()

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.21.3 <4.0.0]")
        self.tool_requires("ninja/[>=1.10.0 <2.0.0]")
