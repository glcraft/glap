includes("xmake/**.lua")
add_rules("mode.debug", "mode.release")
add_requires("libarchive")

includes("qtapp", "consoleapp", "zfiles")

target("console")
    set_kind("phony")
    add_deps("consoleapp", "zfiles")
target("app")
    set_kind("phony")
    add_deps("qtapp", "zfiles")