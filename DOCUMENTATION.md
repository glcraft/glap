# Glap user documentation

## Usage

**Note**: In order to be understand, let's clarify something : In this documentation, *argument* means kind of input the program get, whereas *parameter* is one specific type as as [described below](#argument-type).

### Commands

The program have command(s) defined. Each command have their arguments and none of them is in common, which means if you want common argument in each command, you have to declare them in each command.

To use a command, name the command directly in the beginning of the command line next to the program. For example, use `program_name command1` to select "command1". 

You can define a default command in the parser template argument. If it is set, The first command will be the default one and you donc have to write the name of the command. For example, let admit command1 is the default command, you can use `program_name` to select command1.

Commands can optionally have short name. A short name in the library is one unicode codepoint only, so it can be a ascii letter, a number, a cyrillic letter or even an emoji. Beware that composed unicode symbol like some accented letters or doubled emojis will not work because they are multiple unicode codepoints. So for example, the command1 have a short name 'c', use `program_name c` to select command1 with its short name.

### Arguments

There are 3 types of argument possible :

* **Flags**: argument named without value (e.g. `--flag`)
* **Parameters**: argument named with a value (e.g. `--arg=value`)
* **Inputs**: argument valued without name (e.g. `value`)

Flags and Parameters have to have long name, but it can optionally have short name. In case they have short names, arguments is parsed as `-f` for flags and `-a value` for arguments. 

It's possible to chain short names. For example, `-vvf` would means 2 times the flag -v and one time the flag -f and `-aca value1 value2 value3` means two times the parameter a with value1 and value2 as values, and one time the arg -c with value2 as value. You'll understand it doesn't matter the type of argument the shortname refer to but it is read in order from left to right, so the values will be taken in this order.

Note that the library doesn't take arguments order in account because of its constexpr nature, which means `program command --flag1 --flag2` would have the same result as `program command --flag2 --flag1`.

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
struct Program;
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