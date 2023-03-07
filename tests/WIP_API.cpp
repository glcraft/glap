#include <glap/glap.h>

// Define some arguments
using flag_t = glap::model::Flag<
    glap::Names<"flag", 'f'>
>;
using parameter_t = glap::model::Parameter<
    glap::Names<"param", 'p'>
>;
using flag_verbose_t = glap::model::Flag<
    glap::Names<"verbose", 'v'>
>;
using flag_help_t = glap::model::Flag<
    glap::Names<"help", 'h'>
>;

// Define some commands
using command1_t = glap::model::Command<
    glap::Names<"command1", glap::DISCARD>,
    flag_t
>;

using command2_t = glap::model::Command<
    glap::Names<"command2", glap::DISCARD>,
    parameter_t
>;

// NEW: Define a subcommand in command
// Note: arguments from the parent command are passed in the subcommand. So in this example, subcommand "command1" will 
// have flag_t and parameter_t argument
using command3_t = glap::model::Command<
    glap::Names<"command3", glap::DISCARD>,
    command1_t,
    parameter_t
>;

// NEW: Define a default subcommand in command
// Note: only one default subcommand can be defined.
// Note: parsing command name is priorized over input argument type. So if the input argument is a subcommand name, it 
// will be parsed as a command name, not as an input for the default command.
using command4_t = glap::model::Command<
    glap::Names<"command3", glap::DISCARD>,
    glap::model::Default<command1_t>,
    command2_t,
    parameter_t
>;

// Define a program with two commands
// CHANGE: Second Program template argument doesn't exist anymore. Default command are defined with glap::model::Default
using program1_t = glap::model::Program<
    "program_name",
    command1_t,
    command2_t
>;



// Define a program with two commands and a default command
// NEW: Default command is defined with glap::model::Default. The default command can also be called by name. Of course,
// there can be only one default command.
using program2_t = glap::model::Program<
    "program_name",
    glap::model::Default<command1_t>,
    command2_t
>;

// NEW: Define a program without commands, only arguments
using program3_t = glap::model::Program<
    "program_name",
    flag_verbose_t,
    flag_help_t
>;

// NEW: Define a program with a command and arguments
// Note: arguments from the program are passed in commands. So in this example, verbose and help flags will be passed 
// to commands
using program4_t = glap::model::Program<
    "program_name",
    command1_t,
    command2_t,
    flag_verbose_t,
    flag_help_t
>;

