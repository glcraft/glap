#pragma once
#include "argumentum/inc/optionpack.h"
#include <argumentum/argparse.h>
#include <string>
#include <vector>
struct CompressConfig : public argumentum::CommandOptions
{
    std::vector<std::string> inputs;
    std::string output;

    CompressConfig(std::string_view name) : CommandOptions(name) 
    {}

    void add_parameters( argumentum::ParameterConfig& params ) final
    {
        params.add_parameter( inputs, "input" )
            .minargs( 1 )
            .metavar( "FILES" )
            .help( "File path input" );
        params.add_parameter( output, "--output", "-o" )
            .nargs( 1 )
            .help( "Output file" );
    }
};

struct ListConfig : public argumentum::CommandOptions
{
    std::string input;

    ListConfig(std::string_view name) : CommandOptions(name) 
    {}

    void add_parameters( argumentum::ParameterConfig& params ) final
    {
        params.add_parameter( input, "input" )
            .nargs(1)
            .metavar( "FILE" )
            .help( "File path input" );
    }
};
