includes("xmake/**.lua")
if (is_plat("macosx")) then
    llvm_toolchain("LLVM15.0.0", "macosx")
end

add_rules("mode.debug", "mode.release")
add_requires("fmt 9.0.0", {optional = true}) -- required only if stl has not std::format
add_requires("tl_expected", {optional = true}) -- required only if stl has not std::expected
add_requires("gtest 1.12", {optional = true}) -- required only for glap-tests
target("glap")
    set_kind("$(kind)")
    set_languages("cxx20")
    add_files("src/*.cpp")
    add_headerfiles("include/**.h")
    add_includedirs("include", {public = true})
    on_load(function (target)
        import("lib.detect.check_cxxsnippets")
        local has_std_expected = check_cxxsnippets("#ifndef __cpp_lib_expected\nstatic_assert(false);\n#else\nstatic_assert(__cpp_lib_expected)\n#endif")
            target:add("packages", "fmt", {public = true})
        if not has_std_expected then
            target:add("packages", "tl_expected", {public = true})
        end
    end)

target("glap-example")
    set_kind("binary")
    set_languages("cxx20")
    set_default(false)
    add_deps("glap")
    add_files("tests/example.cpp")
    add_includedirs("include")

target("glap-tests")
    set_kind("binary")
    set_languages("cxx20")
    set_default(false)
    add_deps("glap")
    add_packages("gtest")
    add_files("tests/tests.cpp")
    add_includedirs("include")
    if (is_plat("windows")) then
        add_ldflags("/SUBSYSTEM:CONSOLE")
    end