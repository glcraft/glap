includes("xmake/**.lua")
add_rules("mode.debug", "mode.release")
add_requires("libarchive")
add_requires("fmt")
add_requires("tl_expected")
add_requires("vcpkg::argumentum", {alias = "argumentum"})

includes("qtapp", "consoleapp", "zfiles")