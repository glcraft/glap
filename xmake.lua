includes("xmake/**.lua")

add_rules("mode.debug", "mode.release")
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
    add_headerfiles("include/(**/*.h)", "include/(**/*.inl)")
    add_includedirs("include", {public = true})
    add_options("use_tl_expected", "use_fmt")
    on_load(function (target)
        for _, dep in ipairs({"tl_expected", "fmt"}) do
            if target:opt("use_" .. dep) then
                target:add("packages", dep, {public = true})
            end
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
    add_deps("glap")
    add_packages("gtest")
    add_files("tests/tests.cpp")
    add_includedirs("include")
    set_warnings("allextra", "error")
    add_options("use_tl_expected", "use_fmt", "build_tests")
    if is_plat("linux", "macosx") then
        add_cxflags("-Wno-unknown-pragmas")
    end
    if (is_plat("windows")) then
        add_ldflags("/SUBSYSTEM:CONSOLE")
    end
    on_load(function (target)
        target:set("default", not not target:opt("build_tests"))
    end)
    on_install(function (target)
        -- nothing to install
    end)