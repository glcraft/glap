
target("qtapp")
    add_rules("qt.setup")
    set_kind("binary")
    set_languages("cxxlatest", "clatest")
    add_files("src/*.cpp")
    add_files("*.qrc")
    add_includedirs("include")
    add_deps("zfiles")