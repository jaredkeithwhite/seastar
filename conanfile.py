
#!/usr/bin/env python
# -*- coding: utf-8 -*-

from conans import ConanFile, CMake, tools
import os


class SeastarConan(ConanFile):
    name = "seastar"
    version = "master"
    description = "SeaStar is an event-driven framework allowing you to write non-blocking, asynchronous code in a relatively straightforward manner (once understood)."
    # topics can get used for searches, GitHub topics, Bintray tags etc. Add here keywords about the library
    topics = ("conan", "seastar", "scylladb")
    url = "https://github.com/scylladb/seastar"
    homepage = "https://www.seastar.io"
    author = "Bincrafters <bincrafters@gmail.com>"
    license = "MIT"  # Indicates license type of the packaged library; please use SPDX Identifiers https://spdx.org/licenses/
    exports = ["LICENSE.md"]      # Packages the license for the conanfile.py
    # Remove following lines if the target lib does not use cmake.
    generators = "cmake_paths"

    # Options may need to change depending on the packaged library.
    settings = "os", "arch", "compiler", "build_type"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    # Custom attributes for Bincrafters recipe conventions
    _source_subfolder = "source_subfolder"
    _build_subfolder = "build_subfolder"

    #
    requires = (
        "GnuTLS/3.6.5@waga/thirdparty",
        "hwloc/1.11.10@waga/thirdparty",
        "c-ares/1.14.0@conan/stable",
        "cryptopp/7.0.0@bincrafters/stable",
        "yaml-cpp/0.6.2@bincrafters/stable",
        "zlib/1.2.11@conan/stable",
        "lksctp-tools/1.0.17@bincrafters/stable",
        "libaio/0.3.111@waga/thirdparty",
        "libxml2/2.9.3@bincrafters/stable",
        "libpciaccess/0.14@waga/thirdparty",
        "lz4/1.8.3@waga/thirdparty",
        "fmt/5.2.1@bincrafters/stable",
        "numactl/2.0.12@waga/thirdparty",
        "protoc_installer/3.6.1@bincrafters/stable",
        "protobuf/3.6.1@bincrafters/stable",
        "boost/1.68.0@waga/thirdparty",
        "ragel_installer/6.10@waga/thirdparty",
        "gmp/6.1.2@waga/thirdparty"
    )
