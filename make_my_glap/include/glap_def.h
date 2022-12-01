#include "glap/core/utils.h"
#include "glap/model.h"
#include <glap/glap.h>
#include <filesystem>

namespace args 
{
    inline auto path_exists (std::string_view path) -> bool {
        return std::filesystem::exists(path);
    };

    using param_yaml_t = glap::model::Parameter<glap::Names<"yaml", 'y'>, glap::discard, path_exists>;

    using param_output_t = glap::model::Parameter<glap::Names<"output", 'o'>, glap::discard, glap::discard>;

    using command_generate_t = glap::model::Command<glap::Names<"generate", 'g'>, param_yaml_t, param_output_t>;

    using program_t = glap::model::Program<"make_my_glap", glap::model::DefaultCommand::FirstDefined, command_generate_t>;
}