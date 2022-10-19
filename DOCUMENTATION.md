# Glap user documentation

## Parser model

### Utils

#### Parameter type
```cpp
/// In namespace glap::model
enum class ParameterType {
    Argument,
    Flag,
    Input
};
```

#### Names
```cpp
// In namespace glap
template <StringLiteral LongName, auto ShortName = discard> 
struct Names;
```


### Program templated class
```cpp
/// In namespace glap::model
template<class... Commands>
struct Program
```
### Command templated class
```cpp
/// In namespace glap::model
template <class CommandNames, IsParameter... P>
class Command;
```
### Simple argument templated class
```cpp
/// In namespace glap::model
template <class ArgNames, auto Resolver = discard, auto Validator = discard>
struct Argument;
```
### Multiple arguments templated class
```cpp
/// In namespace glap::model
template <class ArgNames, auto N = discard, auto Resolver = discard, auto Validator = discard>
struct Arguments;
```
### Flag templated class
```cpp
/// In namespace glap::model
template <class ArgNames>
struct Flag;
```
### Single expected input templated class
```cpp
/// In namespace glap::model
template <auto Resolver = discard, auto Validator = discard>
struct Input;
```
### Multiple expected inputs templated class
```cpp
/// In namespace glap::model
template <auto N = discard, auto Resolver = discard, auto Validator = discard>
struct Inputs;
```
### Parser class
```cpp
/// In namespace glap
template<StringLiteral Name, DefaultCommand def_cmd, class... Commands>
class Parser;
```


## Help generation

### Utils

#### Descriptions
```cpp
template<StringLiteral Short>
struct Description;
template<StringLiteral Short, StringLiteral Long>
struct FullDescription;
```

### Program description class
```cpp
/// In namespace glap::help::model
template<StringLiteral Name, IsDescription Desc, class ...Commands>
struct Program;
```
### Command description class
```cpp
/// In namespace glap::help::model
template<StringLiteral Name, IsDescription Desc, class ...Params>
struct Command;
```
### Parameter description class
```cpp
/// In namespace glap::help::model
template<StringLiteral Name, IsDescription Desc> 
struct Parameter;
```
### Help generator class
```cpp
template<class FromHelp, class FromParser> 
struct Help
```