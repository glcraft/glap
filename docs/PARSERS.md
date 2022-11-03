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
    // Returns an expected with the model if success, or an error with the position of
    // the argument which caused the error.
    constexpr auto operator()(utils::Iterable<std::string_view> auto args) const -> PosExpected<OutputType>;
    // Parse the suite of arguments contained in `args`. 
    // `args` is a BiIterator object (a struct containing begin and end iterator)
    // Returns an expected with the model if success, or an error with the position of
    // the argument which caused the error.
    template <class Iter>
    constexpr auto operator()(utils::BiIterator<Iter> args) const -> PosExpected<OutputType>;
    // Parse the suite of arguments contained in `args`. 
    // `args` is a BiIterator object (a struct containing begin and end iterator)
    // `model` is the reference of the output model.
    // Returns an expected with the end parsing itterator if success, or an error with 
    // the position of the argument which caused the error.
    template <class Iter>
    constexpr auto parse(OutputType& model, utils::BiIterator<Iter> args) const -> PosExpected<Iter>;
}
// constexpr instance of the class Parser
template <class ModelType>
static constexpr auto parser = Parser<ModelType>{};
```
### Description

The parser class is 

## Command
```cpp
/// In namespace glap::model
template <class CommandNames, IsArgument... P>
class Command;
```
Model to define a command for the parser
## Single parameter
```cpp
/// In namespace glap::model
template <class ArgNames, auto Resolver = discard, auto Validator = discard>
struct Parameter;
```
## Multiple parameters
```cpp
/// In namespace glap::model
template <class ArgNames, auto N = discard, auto Resolver = discard, auto Validator = discard>
struct Parameters;
```
## Flag templated class
```cpp
/// In namespace glap::model
template <class ArgNames>
struct Flag;
```
## Single expected input
```cpp
/// In namespace glap::model
template <auto Resolver = discard, auto Validator = discard>
struct Input;
```
## Multiple expected inputs
```cpp
/// In namespace glap::model
template <auto N = discard, auto Resolver = discard, auto Validator = discard>
struct Inputs;
```

## Program templated class
```cpp
/// In namespace glap::model
template<StringLiteral Name, DefaultCommand def_cmd, class... Commands>
struct Program;
```

## Names

```cpp
// In namespace glap
template <StringLiteral LongName, auto ShortName = discard> 
struct Names;
```

## Error

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