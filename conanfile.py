from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout
from conan.tools.files import copy, get
import os

class LobEngineConan(ConanFile):
    name = "lob-engine"
    version = "1.0.0"
    license = "Proprietary"
    author = "LOB Engine Contributors"
    url = "https://github.com/example/lob-engine"
    description = "Low-Latency Limit Order Book Engine"
    topics = ("trading", "order-book", "hft", "cpp20")
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_tests": [True, False],
        "with_benchmarks": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "with_tests": True,
        "with_benchmarks": True,
    }

    exports_sources = "CMakeLists.txt", "core/*", "tests/*", "bench/*", "include/*"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        self.requires("googletest/1.14.0")
        self.requires("benchmark/1.8.3")
        self.requires("spdlog/1.12.0")
        self.requires("fmt/10.1.1")

    def build_requirements(self):
        self.tool_requires("cmake/3.28.1")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["BUILD_TESTS"] = self.options.with_tests
        tc.variables["BUILD_BENCHMARKS"] = self.options.with_benchmarks
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(self, "*.h", dst=os.path.join(self.package_folder, "include"), src=self.source_folder, keep_path=False)
        copy(self, "*.a", dst=os.path.join(self.package_folder, "lib"), src=self.build_folder, keep_path=False)
        copy(self, "*.lib", dst=os.path.join(self.package_folder, "lib"), src=self.build_folder, keep_path=False)
        copy(self, "*.so", dst=os.path.join(self.package_folder, "lib"), src=self.build_folder, keep_path=False)
        copy(self, "*.dylib", dst=os.path.join(self.package_folder, "lib"), src=self.build_folder, keep_path=False)
        copy(self, "*.dll", dst=os.path.join(self.package_folder, "bin"), src=self.build_folder, keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["lob_engine"]
        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.system_libs = ["pthread", "rt"]
        elif self.settings.os == "Windows":
            self.cpp_info.system_libs = ["ws2_32", "winmm"]