function get_mmg()
    import("lib.detect.find_tool")
    local mmg = find_tool("mmg")
    if not mmg then
        mmg = {program = "/home/gly/Projets/glap/build/linux/x86_64/release/mmg"}
    end
    if not mmg then
        raise("mmg not found!")
    end
    return mmg.program
end

function mmg_update_file(target, input_file, opt)
    import("core.base.option")
    local opt = opt or {}
    local mmg_program = opt.mmg_program or get_mmg()
    local config_dir = opt.config_dir
    if not config_dir then
        config_dir = path.join(target:configdir(), "mmg_include")
        if not os.isdir(config_dir) then
            os.mkdir(config_dir)
        end
    end

    local output_dir = path.join(config_dir, path.directory(input_file))
    if output_dir ~= "" and not os.isdir(output_dir) then
        os.mkdir(output_dir)
    end
    local output_file = path.join(config_dir, input_file:gsub("%.ya?ml$", ""):gsub("%.glap", "") .. ".h")
    -- generate the header file from the mmg configuration file
    if option.get("verbose") then
        cprint("${dim}generating %s to %s ..", input_file, output_file)
    end
    os.runv(mmg_program, {"-t", "header", "-i", input_file, "-o", output_file})

    

    return output_file
end
function mmg_update_target(target)
    local mmg_program = get_mmg()
    local mmg_sourcebatches = target:sourcebatches()["glap.mmg"]
    if mmg_sourcebatches then
        return
    end
    local config_dir = path.join(target:configdir(), "mmg_include")
    if not os.isdir(config_dir) then
        os.mkdir(config_dir)
    end

    for _, sourcebatch in ipairs(mmg_sourcebatches.sourcefiles) do
        mmg_update_file(target, sourcebatch, {mmg_program = mmg_program, config_dir = config_dir})
    end
    -- path of the target relative to the project directory
    local target_relat = path.relative(target:scriptdir(), os.projectdir())
    local includedirs = path.join(config_dir, target_relat)
    -- add the mmg configuration directory (specific to the target) to the target
    target:add("includedirs", includedirs)
end