-- Setup llvm toolchain for macosx
--
-- @param toolchain_name the toolchain name
--        The toolchain name must be the base name of a xctoolchain directory (e.g "LLVM15.0.0").
-- @param sdk the sdk name
--        The sdk name must be the base name of a sdk directory (e.g. "macosx").
-- @note During the toolchain check, the command xcrun is used to get the sdk and toolchain path.
--       It then stored in config. Once check is complete, xcode environment is avoided.
-- @example
-- in the xmake.lua
-- ```lua
-- includes("llvm.lua")
-- llvm_toolchain("LLVM15.0.0", "macosx")
-- ```
-- then call `xmake f --toolchain=llvm-15.0.0` to use the toolchain.
function llvm_toolchain(toolchain_name, sdk)
    local version = toolchain_name:sub(5) -- 5 = #"LLVM"+1
    toolchain("llvm-"..version)
        set_kind("standalone")
        on_check(function (toolchain)
            -- toolchain only support macosx
            if os.host() ~= "macosx" then
                return false
            end
            -- facility to execute a command and get the output in return
            function exec_return(cmd, argv, opt)
                local opt = opt or {}
                outfile = outfile or os.tmpfile()
                errfile = errfile or os.tmpfile()
                local ok, _ = os.execv(cmd, argv, table.join(opt, {stdout = outfile, stderr = errfile}))
                return ok, os.exists(outfile) and io.readfile(outfile) or nil, os.exists(errfile) and io.readfile(errfile) or nil
            end
            -- setup xcrun arguments
            if toolchain_name then 
                xcrun_args_common = table.join(xcrun_args, {"-toolchain", toolchain_name})
            end
            if sdk then 
                xcrun_args_common = table.join(xcrun_args, {"-sdk", sdk} )
            end
            
            local logfile = os.tmpfile()
            -- get clang path
            local ok, stdout, stderr = exec_return("xcrun", table.join(xcrun_args_common, {"--find", "clang"}))
            if ok ~= 0 then
                print("error running xcrun --find clang:" .. stderr)
                return false
            end
            local clang_path = stdout
            -- get sdk path
            local ok, stdout, stderr = exec_return("xcrun", table.join(xcrun_args_common, {"--show-sdk-path"}))
            if ok ~= 0 then
                print("error running xcrun --show-sdk-path:" .. stderr)
                return false
            end
            local sdk_path = stdout:replace("\n", "")
            -- get sdk platform path
            local ok, stdout, stderr = exec_return("xcrun", table.join(xcrun_args_common, {"--show-sdk-platform-path"}))
            if ok ~= 0 then
                print("error running xcrun --show-sdk-platform-path:" .. stderr)
                return false
            end
            local platform_path = stdout:gsub("\n", "")
            local toolchain_path = path(clang_path):directory():directory():directory()
            -- save config
            toolchain:config_set("toolchain_name", "LLVM"..version)
            toolchain:config_set("toolchain_version", version)
            toolchain:config_set("toolchain_path", toolchain_path:str())
            toolchain:config_set("sdk_path", sdk_path)
            toolchain:config_set("platform_path", platform_path)
            toolchain:config_set("clang_path", clang_path)
            toolchain:configs_save()
            cprint("checking for %s ... ${color.success}%s (%s)", toolchain:name(), toolchain:config("toolchain_name"), toolchain:config("toolchain_path"))
            return true
        end)
        on_load(function(toolchain)

            local clang_path = toolchain:config("clang_path")
            local toolchain_path = toolchain:config("toolchain_path")
            local sdk_path = toolchain:config("sdk_path")
            local platform_path = toolchain:config("platform_path")
            local bin_dir = path(clang_path):directory():str()
            local with_lld = true

            import("lib.detect.find_tool")
            -- setup clang tool (honestly, I don't know if this is needed)
            find_tool("clang", {check = function (tool) os.run("%s -v", tool) end, paths={path.join(toolchain_path, "/usr/bin")}, version=true})
            find_tool("clang++", {check = function (tool) os.run("%s -v", tool) end, paths={path.join(toolchain_path, "/usr/bin")}, version=true})
            -- setup toolsets
            toolchain:set("toolset", "cc", path.join(bin_dir, "clang"))
            toolchain:set("toolset", "cxx", path.join(bin_dir, "clang++"), path.join(bin_dir, "clang"))
            -- toolchain:set("toolset", "ld", "clangxx@" .. path.join(bin_dir, with_lld and "ld64.lld" or "clang"))
            toolchain:set("toolset", "ld", path.join(bin_dir, "clang"))
            toolchain:set("toolset", "sh", path.join(bin_dir, "clang"))
            toolchain:set("toolset", "ar", path.join(bin_dir, "ar"), "ar")
            toolchain:set("toolset", "as", path.join(bin_dir, "clang"))
            toolchain:set("toolset", path.join(bin_dir, "strip"), strip)
            toolchain:set("toolset", "mxx", path.join(bin_dir, "clang++"), path.join(bin_dir, "clang"))
            toolchain:set("toolset", "mm", path.join(bin_dir, "clang"))
            toolchain:set("toolset", "dsymutil", path.join(bin_dir, "dsymutil"), "dsymutil")
            
            -- setup architectures
            local arch = toolchain:arch()
            for _, v in ipairs(arch:split(",")) do
                toolchain:add("cxflags", "-arch", v)
                toolchain:add("ldflags", "-arch", v)
                toolchain:add("shflags", "-arch", v)
            end
            -- setup lld
            if with_lld then
                toolchain:add("ldflags", "-fuse-ld=lld")
                toolchain:add("shflags", "-fuse-ld=lld")
            end

            -- setup cxflags system
            -- the sdk path toolchain is the sysroot...
            toolchain:add("cxflags", "-isysroot" .. sdk_path)
            toolchain:add("ldflags", "-isysroot" .. sdk_path)
            -- ...but the C++ standard library is in the toolchain path
            toolchain:add("cxflags", "-stdlib++-isystem" .. toolchain_path .. "/usr/include/c++/v1")
            
            toolchain:add("ldflags", "-rpath", toolchain_path .. "/usr/lib")
            toolchain:add("ldflags", toolchain_path .. "/usr/lib/libc++.dylib")
            toolchain:add("ldflags", toolchain_path .. "/usr/lib/libc++experimental.a")

            
            -- add system framework search paths
            if os.exists(sdk_path .. "/System/Library/Frameworks") then
                toolchain:add("cxflags", "-iframework" .. sdk_path .. "/System/Library/Frameworks")
                -- toolchain:add("ldflags", "-iframework" .. sdk_path .. "/System/Library/Frameworks")
            end
            if os.exists(sdk_path .. "/System/Library/PrivateFrameworks") then
                toolchain:add("cxflags", "-iframework" .. sdk_path .. "/System/Library/PrivateFrameworks")
                -- toolchain:add("ldflags", "-iframework" .. sdk_path .. "/System/Library/PrivateFrameworks")
            end
            if os.exists(platform_path .. "/Developer/Library/Frameworks") then
                toolchain:add("cxflags", "-iframework" .. platform_path .. "/Developer/Library/Frameworks")
                -- toolchain:add("ldflags", "-iframework" .. platform_path .. "/Developer/Library/Frameworks")
            end
            if os.exists(platform_path .. "/Developer/Library/PrivateFrameworks") then
                toolchain:add("cxflags", "-iframework" .. platform_path .. "/Developer/Library/PrivateFrameworks")
                -- toolchain:add("ldflags", "-iframework" .. platform_path .. "/Developer/Library/PrivateFrameworks")
            end
            
        end)
    toolchain_end()
end

llvm_toolchain("LLVM15.0.0", "macosx")