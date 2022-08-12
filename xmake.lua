includes("xmake/**.lua")
add_rules("mode.debug", "mode.release")

add_requires("libarchive")
add_requires("fmt")
add_requires("tl_expected")

llvm_toolchain("LLVM15.0.0", "macosx")

includes("qtapp", "consoleapp", "zfiles")