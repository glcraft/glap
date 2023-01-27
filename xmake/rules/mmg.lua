rule("glap.mmg")
    set_extensions(".glap.yaml", ".glap.yml")
    on_config(function (target)
        -- import("lib.detect.find_tool")
        -- local mmg = find_tool("mmg")
        local mmg_program = "/home/gly/Projets/glap/build/linux/x86_64/release/mmg"

        if mmg_program then
            local mmg_sourcebatches = target:sourcebatches()["glap.mmg"]
            if mmg_sourcebatches then
                local path_include = path.join(target:configdir(), "mmg_include")
                if not os.isdir(path_include) then
                    os.mkdir(path_include)
                end
                for _, sourcebatch in ipairs(mmg_sourcebatches.sourcefiles) do
                    local sourcedir = path.join(path_include, path.directory(sourcebatch))
                    print("basename: " .. sourcedir)
                    if sourcedir ~= "" and not os.isdir(sourcedir) then
                        os.mkdir(sourcedir)
                    end
                    os.runv(mmg_program, {"-t", "header", "-i", sourcebatch, "-o", path.join(path_include, sourcebatch:gsub("%.glap%.ya?ml", "") .. ".h")})
                end
                -- path of the target relative to the project directory
                local target_relat = path.relative(target:scriptdir(), os.projectdir())
                local path_includes = path.join(path_include, target_relat)
                target:add("includedirs", path_includes)
            end
        end
    end)