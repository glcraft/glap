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

```cpp
/// In namespace glap
template<StringLiteral Name, DefaultCommand def_cmd, class... Commands>
class Parser;
```

### Template arguments

* **Name**: The name of the program
* **def_cmd**: enum value which define if there is a default command of not. It can be `glap::DefaultCommand::FirstDefined` or `glap::DefaultCommand::None`
* **Commands**: list of command classes. Use [Command](#command)

### Functions

```cpp
constexpr auto parse(glap::utils::Iterable<std::string_view> auto args) const -> PosExpected<model::Program<Commands...>>;
```
Parse each argument using an iterable container (It can be a vector, a span, an array...).

The function returns a [Program](#program-templated-class) if the parse success or an [Error](#error) if it fails.

## Command
```cpp
/// In namespace glap::model
template <class CommandNames, IsArgument... P>
class Command;
```
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
template<class... Commands>
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
using multi_param_t = glap::model::Parameters<
    glap::Names<"multi_param", 'm'>
    0, // no limit
>;
using inputs_t = glap::model::Inputs<
    0 // no limit
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

using parser_t = glap::Parser<"myprogram", glap::FirstDefined, command1_t, command2_t>;

int main(int argc, char** argv)
{
    auto args = std::span{argv, argv+argc};
    parser_t parser;
    auto result = parser.parse(args);
}
```