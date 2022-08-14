add_requires("fmt", {optional = true})
add_requires("tl_expected", {optional = true})
target("glap")
    set_kind("static")
    set_languages("cxx20")
    add_files("src/*.cpp")
    add_headerfiles("include/**.h")
    add_includedirs("include")
    on_load(function (target)
        import("lib.detect.check_cxsnippets")
        local has_std_format = check_cxsnippets("static_assert(__cpp_lib_format >= 201907L)")
        local has_std_expected = check_cxsnippets("static_assert(__cpp_lib_expected)")
        if not has_std_format then
            target:add("packages", "fmt", {public = true})
        end
        if not has_std_expected then
            target:add("packages", "tl_expected", {public = true})
        end
    end)

-- target("test")
--     set_kind("binary")
--     add_deps("glap")
--     add_files("test/*.cpp")
--     add_includedirs("include")