function mmg_update_target (target)
    import("lib.detect.find_program")
    local mmg_program = find_program("mmg")
    if not mmg_program then
        mmg_program = "/home/gly/Projets/glap/build/linux/x86_64/release/mmg"
    end

    if not mmg_program then
        raise("mmg not found!")
    end
    -- mmg source batches of the target
    local mmg_sourcebatches = target:sourcebatches()["glap.mmg"]
    if mmg_sourcebatches then
        return
    end
    -- path of the mmg configuration directory
    local config_path = path.join(target:configdir(), "mmg_include")
    if not os.isdir(config_path) then
        os.mkdir(config_path)
    end

    for _, sourcebatch in ipairs(mmg_sourcebatches.sourcefiles) do
        local configdir = path.join(config_path, path.directory(sourcebatch))
        -- create the mmg output directory if it does not exist
        if configdir ~= "" and not os.isdir(configdir) then
            os.mkdir(configdir)
        end
        -- generate the header file from the mmg configuration file
        os.runv(mmg_program, {"-t", "header", "-i", sourcebatch, "-o", path.join(config_path, sourcebatch:gsub("%.glap%.ya?ml", "") .. ".h")})
    end
    -- path of the target relative to the project directory
    local target_relat = path.relative(target:scriptdir(), os.projectdir())
    local includedirs = path.join(config_path, target_relat)
    -- add the mmg configuration directory (specific to the target) to the target
    target:add("includedirs", includedirs)
end

rule("glap.mmg")
    set_extensions(".glap.yaml", ".glap.yml")
    on_config(mmg_update_target)
    before_build(mmg_update_target)