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
    set_showmenu(true)
    set_description("Use tl::expected instead of std::expected")
    add_defines("GLAP_USE_TL_EXPECTED", {public = true})
    add_deps("enable_std_module")
    on_check(function (option)
        if option:dep("enable_std_module"):enabled() then
            option:enable(false)
        else
            option:enable(true)
        end
    end)
option("use_fmt")
    set_showmenu(true)
    set_description("Use fmt instead of std::format")
    add_defines("GLAP_USE_FMT", {public = true})
    add_deps("enable_module")
    on_check(function (option)
        if option:dep("enable_module"):enabled() then
            option:enable(false)
        else
            option:enable(true)
        end
    end)
option("enable_module")
    set_default(false)
    set_showmenu(true)
    set_description("Enable C++20 module support")
    add_defines("GLAP_USE_MODULE", {public = true})
option("enable_std_module")
    set_default(false)
    add_deps("enable_module")
    set_showmenu(true)
    set_description("Enable C++23 std module support")
    add_defines("GLAP_USE_STD_MODULE", {public = true})

target("glap")
    set_kind("$(kind)")
    if has_config("enable_std_module") or not has_config("use_tl_expected") then
        set_languages("cxxlatest")
    else
        set_languages("cxx20")
    end
    add_files("src/*.cpp")
    add_headerfiles("include/(glap/**.h)", "include/(glap/**.inl)")
    add_includedirs("include", {public = true})
    add_options("use_tl_expected", "use_fmt", "enable_std_module")
    add_configfiles("xmake/config/config.h.in")
    if has_config("use_tl_expected") then
        add_packages("tl_expected", {public = true})
    end
    if has_config("use_fmt") then
        add_packages("fmt", {public = true})
    end
    if has_config("enable_module") then
        add_files("modules/**.mpp")
    end
    on_load(function(target)
        if not has_config("enable_std_module") then
            -- target:data_set("c++.msvc.enable_std_import", false)
        end
    end)

target("glap-example")
    set_kind("binary")
    if not has_config("use_tl_expected") then
        set_languages("cxxlatest")
    else
        set_languages("cxx20")
    end
    set_default(false)
    add_deps("glap")
    add_files("tests/example.cpp")
    add_options("use_tl_expected", "use_fmt", "enable_module", "enable_std_module")

if has_config("enable_module") then
    target("glap-module-example")
        set_kind("binary")
        if has_config("enable_std_module") or not has_config("use_tl_expected") then
            set_languages("cxxlatest")
        else
            set_languages("cxx20")
        end
        add_defines("GLAP_USE_MODULE")
        set_default(false)
        add_deps("glap")
        add_files("tests/example.cpp")
        add_options("use_tl_expected", "use_fmt", "enable_std_module")
        on_load(function(target)
            if not has_config("enable_std_modules") then
                target:data_set("c++.msvc.enable_std_import", false)
            end
        end)
end

if has_config("build_tests") then
    target("glap-tests")
        set_kind("binary")
        if not has_config("use_tl_expected") then
            set_languages("cxxlatest")
        else
            set_languages("cxx20")
        end
        add_deps("glap")
        add_packages("gtest")
        add_files("tests/tests.cpp")
        set_warnings("allextra", "error")
        add_options("use_tl_expected", "use_fmt")
        if is_plat("linux", "macosx") then
            add_cxflags("-Wno-unknown-pragmas")
        end
        if is_plat("windows") then
            add_ldflags("/SUBSYSTEM:CONSOLE")
        end
        on_install(function (target)
            -- nothing to install
        end)

    if has_config("enable_module") then
        target("glap-tests-modules")
            set_kind("binary")
            if has_config("enable_std_module") or not has_config("use_tl_expected") then
                set_languages("cxxlatest")
            else
                set_languages("cxx20")
            end
            add_deps("glap")
            add_packages("gtest")
            add_files("tests/tests.cpp")
            set_warnings("allextra", "error")
            add_options("use_tl_expected", "use_fmt", "enable_std_module")
            add_defines("GLAP_USE_MODULE")
            if is_plat("linux", "macosx") then
                add_cxflags("-Wno-unknown-pragmas")
            end
            if is_plat("windows") then
                add_ldflags("/SUBSYSTEM:CONSOLE")
            end
            on_load(function(target)
                if not has_config("enable_std_modules") then
                    target:data_set("c++.msvc.enable_std_import", false)
                end
            end)
            on_install(function (target)
                -- nothing to install
            end)
    end
end