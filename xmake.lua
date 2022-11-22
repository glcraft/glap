includes("xmake/**.lua")

add_rules("mode.debug", "mode.release")
add_requires("fmt 9.0.0", {optional = true}) -- required only if stl has not std::format
add_requires("tl_expected", {optional = true}) -- required only if stl has not std::expected
add_requires("gtest 1.12", {optional = true}) -- required only for glap-tests
    
option("use_tl_expected")
    set_default(true)
    set_showmenu(true)
    set_configvar("GLAP_USE_TL_EXPECTED", true)
option("use_fmt")
    set_default(true)
    set_showmenu(true)
    set_configvar("GLAP_USE_FMT", true)

target("glap")
    set_kind("$(kind)")
    set_languages("cxx20")
    add_files("src/*.cpp")
    add_headerfiles("include/**.h")
    add_includedirs("include", {public = true})
    add_options("use_tl_expected", "use_fmt")
    add_configfiles("xmake/config/config.h.in")
    on_load(function (target)
        if target:opt("use_tl_expected") then 
            target:add("packages", "tl_expected", {public = true})
        end
        if target:opt("use_fmt") then 
            target:add("packages", "fmt", {public = true})
        end
    end)

target("glap-example")
    set_kind("binary")
    set_languages("cxx20")
    set_default(false)
    add_deps("glap")
    add_files("tests/example.cpp")
    add_options("use_tl_expected", "use_fmt")
    add_includedirs("include")

target("glap-tests")
    set_kind("binary")
    set_languages("cxx20")
    set_default(false)
    add_deps("glap")
    add_packages("gtest")
    add_files("tests/tests.cpp")
    add_includedirs("include")
    set_warnings("allextra", "error")
    if is_plat("linux", "macosx") then
        add_cxflags("-Wno-unknown-pragmas")
    end
    add_options("use_tl_expected", "use_fmt")
    if (is_plat("windows")) then
        add_ldflags("/SUBSYSTEM:CONSOLE")
    end