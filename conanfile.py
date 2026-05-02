from conan import ConanFile


class HerculesRecipe(ConanFile):
    name = "hercules"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeConfigDeps"

    # Custom options for hercules
    options = {
        "with_libbacktrace": [True, False]
    }
    default_options = {
        "with_libbacktrace": False
    }

    def requirements(self):
        if self.options.with_libbacktrace:
            self.requires("libbacktrace/cci.20240730")
        self.requires("cjson/1.7.19")
        self.requires("mariadb-connector-c/3.4.8", options={"shared": True})
        self.requires("pcre/8.45")
        self.requires("zlib/1.3.2")
