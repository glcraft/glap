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
            target:add("packages", "fmt")
        end
        if not has_std_expected then
            target:add("packages", "tl_expected")
        end
    end)
