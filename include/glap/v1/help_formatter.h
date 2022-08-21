#pragma once
#include <fmt/format.h>
#include "fmt/core.h"
#include "config.h"
namespace glap 
{
    inline auto default_help_formatter() {
        return [](const config::Command& command) {
            fmt::print("{}", command.longname);
        };
    }
}