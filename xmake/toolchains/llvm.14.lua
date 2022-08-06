toolchain("llvm-14")
    set_kind("standalone")
    on_check(function (toolchain)
        import("lib.detect.find_path")
        import("lib.detect.find_tool")

        local found_custom_llvm = find_path("usr/bin/clang", {"$(env HOME)/Library/Developer/Toolchains/LLVM14.*.xctoolchain", "/Library/Developer/Toolchains/LLVM14.*.xctoolchain"})
        if not found_custom_llvm then
            return false
        end

        local toolchain_name = path.basename(found_custom_llvm)
        local tool = find_tool("clang", {check = "--help", program = "xcrun -toolchain " .. toolchain_name .. " -sdk macosx clang"})
        if tool then
            toolchain:config_set("toolchain_name", toolchain_name)
            toolchain:config_set("toolchain_path", found_custom_llvm)
            toolchain:configs_save()
            return true
        end

        return false
    end)
    on_load(function(toolchain)
        local toolchain_name = toolchain:config("toolchain_name")
        local cross = "xcrun -toolchain " .. toolchain_name .. " -sdk macosx "
        -- print("cross: " .. cross)
        toolchain:set("toolset", "cc", cross .. "clang")
        toolchain:set("toolset", "cxx", cross .. "clang", cross .. "clang++")
        toolchain:set("toolset", "ld", cross .. "clang++", cross .. "clang")
        toolchain:set("toolset", "sh", cross .. "clang++", cross .. "clang")
        toolchain:set("toolset", "ar", cross .. "ar")
        toolchain:set("toolset", "strip", cross .. "strip")
        toolchain:set("toolset", "dsymutil", cross .. "dsymutil", "dsymutil")
        toolchain:set("toolset", "mm", cross .. "clang")
        toolchain:set("toolset", "mxx", cross .. "clang", cross .. "clang++")
        toolchain:set("toolset", "sc", cross .. "swiftc", "swiftc")
        toolchain:set("toolset", "scld", cross .. "swiftc", "swiftc")
        toolchain:set("toolset", "scsh", cross .. "swiftc", "swiftc")
    end)