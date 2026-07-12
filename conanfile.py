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
        ssl_lib = "openssl"
        if self.settings.os == "Windows":
            ssl_lib = "schannel"

        if self.options.with_libbacktrace:
            self.requires("libbacktrace/cci.20240730")
        self.requires("cjson/1.7.19")
        self.requires("mariadb-connector-c/3.4.8", options={"shared": True, "with_ssl": ssl_lib})
        self.requires("pcre/8.45")
        self.requires("zlib/1.3.2")
        self.requires("giflib/5.2.2")
