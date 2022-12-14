includes("xmake/**.lua")

add_rules("mode.debug", "mode.release", "mode.asan")
add_requires("fmt 9.0.0", {optional = true}) -- required only if stl has not std::format
add_requires("tl_expected", {optional = true}) -- required only if stl has not std::expected
add_requires("gtest 1.12", {optional = true}) -- required only for glap-tests
    
option("build_tests")
    set_default(false)
    set_showmenu(true)
    set_description("Build tests")
option("use_tl_expected")
    set_default(true)
    set_showmenu(true)
    set_description("Use tl::expected instead of std::expected")
    add_defines("GLAP_USE_TL_EXPECTED", {public = true})
option("use_fmt")
    set_default(true)
    set_showmenu(true)
    set_description("Use fmt instead of std::format")
    add_defines("GLAP_USE_FMT", {public = true})

target("glap")
    set_kind("$(kind)")
    set_languages("cxx20")
    add_files("src/*.cpp")
    add_headerfiles("include/(glap/**.h)", "include/(glap/**.inl)")
    add_includedirs("include", {public = true})
    add_options("use_tl_expected", "use_fmt")
    add_configfiles("xmake/config/config.h.in")
    if has_config("use_tl_expected") then
        add_packages("tl_expected", {public = true})
    end
    if has_config("use_fmt") then
        add_packages("fmt", {public = true})
    end

target("glap-example")
    set_kind("binary")
    set_languages("cxx20")
    set_default(false)
    add_deps("glap")
    add_files("tests/example.cpp")
    add_options("use_tl_expected", "use_fmt")

if has_config("build_tests") then
    target("glap-tests")
        set_kind("binary")
        set_languages("cxx20")
        add_deps("glap")
        add_packages("gtest")
        add_files("tests/tests.cpp")
        set_warnings("allextra", "error")
        add_options("use_tl_expected", "use_fmt")
        if is_plat("linux", "macosx") then
            add_cxflags("-Wno-unknown-pragmas")
        end
        if (is_plat("windows")) then
            add_ldflags("/SUBSYSTEM:CONSOLE")
        end
        on_install(function (target)
            -- nothing to install
        end)
end