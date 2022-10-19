# Glap user documentation

## Note before reading

### Argument type

In order to be understand, let's clarify something : In this documentation, *argument* means kind of input the program get, whereas *parameter* is one specific type as as described below.

There are 3 types of argument possible :

* **Flags**: argument named without value (e.g. `--flag`)
* **Parameters**: argument named with a value (e.g. `--arg=value`)
* **Inputs**: argument valued without name (e.g. `value`)

## Parser model

### Utils

Here is some util classes to work with the parser model.

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
template <class CommandNames, IsArgument... P>
class Command;
```
### Simple parameter templated class
```cpp
/// In namespace glap::model
template <class ArgNames, auto Resolver = discard, auto Validator = discard>
struct Parameter;
```
### Multiple parameters templated class
```cpp
/// In namespace glap::model
template <class ArgNames, auto N = discard, auto Resolver = discard, auto Validator = discard>
struct Parameters;
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
### Argument description class
```cpp
/// In namespace glap::help::model
template<StringLiteral Name, IsDescription Desc> 
struct Argument;
```
### Help generator class
```cpp
template<class FromHelp, class FromParser> 
struct Help
```