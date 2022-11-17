# Help text generator

It is currently in development, not usable.

## Utils

## Descriptions
```cpp
template<StringLiteral Short>
struct Description;
template<StringLiteral Short, StringLiteral Long>
struct FullDescription;
```

## Program description class
```cpp
/// In namespace glap::help::model
template<StringLiteral Name, IsDescription Desc, class ...Commands>
struct Program;
```
## Command description class
```cpp
/// In namespace glap::help::model
template<StringLiteral Name, IsDescription Desc, class ...Params>
struct Command;
```
## Argument description class
```cpp
/// In namespace glap::help::model
template<StringLiteral Name, IsDescription Desc> 
struct Argument;
```
## Help generator class
```cpp
template<class FromHelp, class FromParser> 
struct Help
```