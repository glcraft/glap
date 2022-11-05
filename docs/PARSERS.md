# Parser model

- [Parser model](#parser-model)
  - [Parser](#parser)
    - [Template arguments](#template-arguments)
    - [Functions](#functions)
  - [Command](#command)
  - [Single parameter](#single-parameter)
  - [Multiple parameters](#multiple-parameters)
  - [Flag templated class](#flag-templated-class)
  - [Single expected input](#single-expected-input)
  - [Multiple expected inputs](#multiple-expected-inputs)
  - [Program templated class](#program-templated-class)
  - [Names](#names)
  - [Error](#error)
  - [Discard](#discard)
  - [Quick example](#quick-example)

## Parser

### Definition

```cpp
/// In namespace glap
template <class ModelType>
class Parser {
    // Parse the suite of arguments contained in `args`. 
    // `args` is a range (container or view with begin/end function members)
    // Returns an expected with the model if success, or an error with the position of the argument which caused the 
    // error.
    constexpr auto operator()(utils::Iterable<std::string_view> auto args) const -> PosExpected<OutputType>;
    // Parse the suite of arguments contained in `args`. 
    // `args` is a BiIterator object (a struct containing begin and end iterator)
    // Returns an expected with the model if success, or an error with the position of the argument which caused the 
    // error.
    template <class Iter>
    constexpr auto operator()(utils::BiIterator<Iter> args) const -> PosExpected<OutputType>;
    // Parse the suite of arguments contained in `args`. 
    // `args` is a BiIterator object (a struct containing begin and end iterator)
    // `model` is the reference of the output model.
    // Returns an expected with the end parsing itterator if success, or an error with the position of the argument 
    // which caused the error.
    template <class Iter>
    constexpr auto parse(OutputType& model, utils::BiIterator<Iter> args) const -> PosExpected<Iter>;
}
// constexpr instance of the class Parser
template <class ModelType>
static constexpr auto parser = Parser<ModelType>{};
```
### Description

The parser class is the main part of the library. 
By using the model you define previously, you'll be able to parse the command line.

The parser expects primarily the Program model class, but it also works with the Command model class (could be useful 
when you don't want any command). In that case, skip the first command line argument reserved to the program name. 
Otherwise, it could be parsed as an input of the "command".

## Program

### Definition

```cpp
/// In namespace glap::model
template<StringLiteral Name, DefaultCommand def_cmd, class... Commands>
struct Program {
    static constexpr std::string_view name = Name;
    static constexpr auto default_command = def_cmd;
    std::string_view program;
    std::variant<Commands...> command;
};
```

### Description

Model to define the program command line. 

`Name` is the program name. It does not infer to the parser butt is here to name the program in the 
help. See [HELP.md](./HELP.md) for more details.

`def_cmd` is the default command behaviour which define the default command to parse when you don't specify in the 
command line. There are two values :
* `DefaultCommand::FirstDefined` : The first command defined in `Command` is tthe default program command
* `DefaultCommand::None` : No default command is defined.

In case no default command is defined, if no command is parsed in the command line, an error is raised during the parse.

`Commands` are the models of commands. See the [Command model class](#command) to define them. The long name of each 
command has to be unique. Same goes for the short name. A compile error is raised if not.

Once the command line is parsed, the result of the command is stored in `command` field and the program name (first 
command line argument) is stored in `program` field.

## Command

### Definition

```cpp
/// In namespace glap::model
template <class CommandNames, IsArgument... Arguments>
struct Command {
    using Params = std::tuple<Arguments...>;
    Params arguments;
    // Get the instance of an argument stored in `arguments` by its name. A compile error is raised 
    // 
    template <StringLiteral lit>
    constexpr const auto& get_argument() const noexcept;
    // Get the instance of the input stored in `arguments`. Only if 
    constexpr const auto& get_inputs() const noexcept;
}
```

### Description

Model to define a command for the program command line. 

`CommandNames` is a [Names](#names) model type. 

`Arguments` is a list of arguments for the command. The long name of each argument has to be unique.same goes for the 
short name. A compile error is raised if not. Also, Arguments have to have only one input type.

## Single parameter argument
```cpp
/// In namespace glap::model
template <class ArgNames, auto Resolver = discard, auto Validator = discard>
struct Parameter;
```
## Multiple parameters argument
```cpp
/// In namespace glap::model
template <class ArgNames, auto N = discard, auto Resolver = discard, auto Validator = discard>
struct Parameters;
```
## Flag argument
```cpp
/// In namespace glap::model
template <class ArgNames>
struct Flag;
```
## Single expected input argument
```cpp
/// In namespace glap::model
template <auto Resolver = discard, auto Validator = discard>
struct Input;
```
## Multiple expected inputs argument
```cpp
/// In namespace glap::model
template <auto N = discard, auto Resolver = discard, auto Validator = discard>
struct Inputs;
```

## Names

```cpp
// In namespace glap
template <StringLiteral LongName, auto ShortName = discard> 
struct Names;
```

## Handle errors

## Discard

## Quick example

```cpp
#include <glap/glap.h>
#include <span>

using flag_t = glap::model::Flag<
    glap::Names<"flag", 'f'>
>;
using verbose_t = glap::model::Flag<
    glap::Names<"verbose", 'v'>
>;
using help_t = glap::model::Flag<
    glap::Names<"help", 'h'>
>;
using single_param_t = glap::model::Parameter<
    glap::Names<"single_param", 's'>
>;
using single_int_param_t = glap::model::Parameter<
    glap::Names<"to_int", glap::discard>,
    [] (std::string_view v) -> glap::expected<int, glap::Discard> { 
        try {
            return std::stoi(std::string(v)); 
        } catch(...) {
            // In case stoi results in an error by exception, 
            // we return an error the parser can intercept
            return glap::make_unexpected(glap::discard);
        }
    }
>;
using multi_param_t = glap::model::Parameters<
    glap::Names<"multi_param", 'm'>
    glap::discard, // optional, = no limit
>;
using inputs_t = glap::model::Inputs<
    glap::discard // optional, = no limit
>;

using command1_t = glap::model::Command<
    glap::Names<"command1", 'c'>, 
    flag_t, 
    verbose_t, 
    help_t, 
    inputs_t
>;
using command2_t = glap::model::Command<
    glap::Names<"command2">, // notice that there is no short name here
    single_param_t, 
    multi_param_t, 
    help_t, 
    inputs_t
>;

using program_t = glap::model::Program<"myprogram", glap::model::DefaultCommand::FirstDefined, command1_t, command2_t>;

int main(int argc, char** argv)
{
    auto args = std::span{argv, argv+argc};
    
    auto result = glap::parser(args);
}
```